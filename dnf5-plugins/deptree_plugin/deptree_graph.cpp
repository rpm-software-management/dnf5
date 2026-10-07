// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "deptree_graph.hpp"

#include <fnmatch.h>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/format.hpp>

#include <algorithm>
#include <deque>
#include <iostream>
#include <utility>

namespace dnf5 {

GraphBuilder::GraphBuilder(
    libdnf5::Base & base,
    const std::set<std::string> & types,
    const std::set<std::string> & arches,
    bool reverse,
    bool show_requires,
    bool show_duplicates,
    int depth)
    : base(base),
      available(base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES),
      types(types),
      arches(arches),
      reverse(reverse),
      show_requires(show_requires),
      show_duplicates(show_duplicates),
      max_depth(depth) {
    if (!arches.empty()) {
        const std::vector<std::string> architecture_patterns(arches.begin(), arches.end());
        available.filter_arch(architecture_patterns);
    }
    if (reverse && !show_duplicates) {
        latest_available.emplace(available);
        latest_available->filter_latest_evr();
    }
}

Graph GraphBuilder::build(const libdnf5::rpm::PackageQuery & root_packages) {
    for (const auto & root : root_packages) {
        // The NEVRA-derived node ID deduplicates roots resolved from different specs.
        graph.roots.insert(add_package(root));
    }
    // Seed all roots before traversal so each starts at dependency depth zero.
    std::deque<std::pair<std::string, int>> pending;
    for (const auto & root : graph.roots) {
        pending.emplace_back(root, 0);
    }
    while (!pending.empty()) {
        auto [node_id, level] = std::move(pending.front());
        pending.pop_front();
        // Record boundary nodes min_depth too, even though their dependencies
        // are not expanded.
        if (!graph.min_depths.emplace(node_id, level).second) {
            continue;
        }
        if (max_depth >= 0 && level >= max_depth) {
            continue;
        }
        expand(node_id);
        for (const auto & edge : graph.edges[node_id]) {
            if (graph.nodes.at(edge.target).type == Node::Type::PACKAGE) {
                pending.emplace_back(edge.target, level + 1);
            } else {
                // Structural nodes (non-package) consume no depth. They go to the front
                // of deque to keep processing order by package depth.
                pending.emplace_front(edge.target, level);
            }
        }
    }
    return std::move(graph);
}

std::string GraphBuilder::add_package(const libdnf5::rpm::Package & package) {
    const auto id = "pkg:" + package.get_nevra();
    graph.nodes.emplace(id, Node{Node::Type::PACKAGE, package.get_nevra(), package});
    return id;
}

std::string GraphBuilder::add_node(Node::Type type, const std::string & label, const std::string & identity) {
    const char * prefix = type == Node::Type::CHOICE        ? "choice:"
                          : type == Node::Type::CONDITIONAL ? "cond:"
                          : type == Node::Type::ELSE_BRANCH ? "else:"
                          : type == Node::Type::UNSATISFIED ? "unsatisfied:"
                                                            : "requirement:";
    const auto id = std::string(prefix) + (identity.empty() ? label : identity);
    graph.nodes.emplace(id, Node{type, label, std::nullopt});
    return id;
}

void GraphBuilder::add_edge(const std::string & source, const std::string & target, std::string requirement) {
    auto & outgoing = graph.edges[source];
    auto existing =
        std::find_if(outgoing.begin(), outgoing.end(), [&](const Edge & edge) { return edge.target == target; });
    if (existing == outgoing.end()) {
        outgoing.push_back({target, {}});
        existing = std::prev(outgoing.end());
    }
    if (!requirement.empty() && std::find(existing->requirements.begin(), existing->requirements.end(), requirement) ==
                                    existing->requirements.end()) {
        existing->requirements.push_back(std::move(requirement));
    }
}

std::vector<libdnf5::rpm::Package> GraphBuilder::normalize_candidates(
    std::vector<libdnf5::rpm::Package> candidates) const {
    // Sort for stable output, then keep one package per displayed NEVRA
    // regardless of its solver id.
    std::sort(candidates.begin(), candidates.end(), libdnf5::rpm::cmp_nevra<libdnf5::rpm::Package>);
    candidates.erase(
        std::unique(
            candidates.begin(),
            candidates.end(),
            [](const auto & left, const auto & right) { return left.get_nevra() == right.get_nevra(); }),
        candidates.end());
    return candidates;
}

std::vector<libdnf5::rpm::Package> GraphBuilder::providers(const libdnf5::rpm::Reldep & dependency, bool latest) {
    libdnf5::rpm::PackageQuery query(available);
    query.filter_provides(dependency);
    // Select the latest version only among packages that satisfy this dependency.
    // A globally latest package may not satisfy a versioned capability.
    if (latest && !show_duplicates) {
        query.filter_latest_evr();
    }
    std::vector<libdnf5::rpm::Package> result;
    for (const auto & package : query) {
        result.push_back(package);
    }
    return normalize_candidates(std::move(result));
}

std::vector<libdnf5::rpm::Package> GraphBuilder::latest_candidates(
    std::vector<libdnf5::rpm::Package> candidates) const {
    if (show_duplicates) {
        return normalize_candidates(std::move(candidates));
    }
    libdnf5::rpm::PackageSet candidate_set(base);
    for (const auto & candidate : candidates) {
        candidate_set.add(candidate);
    }
    libdnf5::rpm::PackageQuery query(candidate_set);
    query.filter_latest_evr();
    candidates.clear();
    for (const auto & candidate : query) {
        candidates.push_back(candidate);
    }
    return normalize_candidates(std::move(candidates));
}

void GraphBuilder::link_candidates(
    const std::string & parent,
    const std::string & requirement,
    const std::vector<libdnf5::rpm::Package> & candidates,
    const std::string & choice_label) {
    if (candidates.empty()) {
        const auto unsatisfied_id = add_node(Node::Type::UNSATISFIED, requirement);
        add_edge(parent, unsatisfied_id, requirement);
        return;
    }
    if (show_requires) {
        const auto requirement_id = add_node(Node::Type::REQUIREMENT, requirement);
        add_edge(parent, requirement_id, requirement);
        for (const auto & candidate : candidates) {
            add_edge(requirement_id, add_package(candidate), {});
        }
        return;
    }
    if (candidates.size() == 1) {
        add_edge(parent, add_package(candidates.front()), requirement);
        return;
    }
    const auto choice_id = add_node(Node::Type::CHOICE, choice_label.empty() ? requirement : choice_label);
    add_edge(parent, choice_id, requirement);
    for (const auto & candidate : candidates) {
        add_edge(choice_id, add_package(candidate), {});
    }
}

void GraphBuilder::link_requirement(const std::string & parent, const libdnf5::rpm::Reldep & dependency) {
    const auto dep_text = dependency.to_string();
    // rpmlib dependencies describe RPM feature support, not package dependencies.
    if (dep_text.rfind("rpmlib(", 0) == 0) {
        return;
    }
    const auto op = dependency.get_operator();
    using ReldepOperator = libdnf5::rpm::Reldep::ReldepOperator;
    const auto operands = dependency.get_operands();
    if (operands.empty() || op == ReldepOperator::EQ || op == ReldepOperator::GT || op == ReldepOperator::GTE ||
        op == ReldepOperator::LT || op == ReldepOperator::LTE) {
        // Plain and versioned requirements are satisfied by their direct providers.
        link_candidates(parent, dep_text, providers(dependency));
        return;
    }

    // Rich dependencies always have left and right operands.
    const auto left = operands.get(0);
    const auto right = operands.get(1);
    if (op == ReldepOperator::AND) {
        // Both operands are required, so preserve them as sibling branches.
        link_requirement(parent, left);
        link_requirement(parent, right);
    } else if (op == ReldepOperator::OR) {
        // Either operand can satisfy the relation. Resolve each branch recursively:
        // an operand can itself be a rich dependency, for which a direct provider
        // query would not find its leaf providers.
        const auto choice_id = add_node(Node::Type::CHOICE, dep_text);
        add_edge(parent, choice_id, dep_text);
        link_requirement(choice_id, left);
        link_requirement(choice_id, right);
    } else if (op == ReldepOperator::IF || op == ReldepOperator::UNLESS) {
        // libsolv represents `(A if B else C)` as `IF(A, ELSE(B, C))`.
        if (right.get_operator() != ReldepOperator::ELSE) {
            if (op == ReldepOperator::IF) {
                // A applies only when B is present, so show A below an `if B` branch.
                const auto conditional_id = add_node(Node::Type::CONDITIONAL, right.to_string(), dep_text);
                add_edge(parent, conditional_id, {});
                link_requirement(conditional_id, left);
            } else {
                // For `A unless B`, A is the active dependency; B only suppresses it.
                link_requirement(parent, left);
            }
            return;
        }

        // Keep the condition and its two alternatives as explicit graph branches.
        const auto else_operands = right.get_operands();
        const auto condition = else_operands.get(0);
        const auto alternate = else_operands.get(1);
        const auto conditional_id = add_node(Node::Type::CONDITIONAL, condition.to_string(), dep_text);
        const auto else_id = add_node(Node::Type::ELSE_BRANCH, condition.to_string(), dep_text);
        add_edge(parent, conditional_id, {});
        if (op == ReldepOperator::IF) {
            // `A if B else C`: choose A when B holds and C otherwise.
            link_requirement(conditional_id, left);
            add_edge(conditional_id, else_id, {});
            link_requirement(else_id, alternate);
        } else {
            // `A unless B else C`: choose C when B holds and A otherwise.
            link_requirement(conditional_id, alternate);
            add_edge(conditional_id, else_id, {});
            link_requirement(else_id, left);
        }
    } else if (op == ReldepOperator::WITH || op == ReldepOperator::WITHOUT) {
        // WITH keeps providers satisfying both operands
        // WITHOUT removes providers satisfying the right operand.
        auto candidates = providers(left, false);
        const auto other = providers(right, false);
        candidates.erase(
            std::remove_if(
                candidates.begin(),
                candidates.end(),
                [&](const auto & candidate) {
                    const bool also_provides = std::any_of(other.begin(), other.end(), [&](const auto & item) {
                        return item.get_nevra() == candidate.get_nevra();
                    });
                    return op == ReldepOperator::WITH ? !also_provides : also_provides;
                }),
            candidates.end());
        candidates = latest_candidates(std::move(candidates));
        link_candidates(parent, dep_text, candidates, dep_text);
    } else {
        // Preserve unrecognized operators as one provider-resolved dependency.
        link_candidates(parent, dep_text, providers(dependency));
    }
}

libdnf5::rpm::ReldepList GraphBuilder::dependencies(const libdnf5::rpm::Package & package) const {
    libdnf5::rpm::ReldepList result(base);
    for (const auto & type : types) {
        if (type == "requires") {
            auto list = package.get_requires();
            result.append(list);
        } else if (type == "recommends") {
            auto list = package.get_recommends();
            result.append(list);
        } else if (type == "suggests") {
            auto list = package.get_suggests();
            result.append(list);
        } else if (type == "supplements") {
            auto list = package.get_supplements();
            result.append(list);
        } else if (type == "enhances") {
            auto list = package.get_enhances();
            result.append(list);
        }
    }
    return result;
}

void GraphBuilder::expand_reverse(const std::string & node_id, const libdnf5::rpm::Package & package) {
    libdnf5::rpm::PackageSet target(base);
    target.add(package);
    libdnf5::rpm::PackageQuery dependents(base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);
    for (const auto & type : types) {
        libdnf5::rpm::PackageQuery query(show_duplicates ? available : latest_available.value());
        if (type == "requires") {
            query.filter_requires(target);
        } else if (type == "recommends") {
            query.filter_recommends(target);
        } else if (type == "suggests") {
            query.filter_suggests(target);
        } else if (type == "supplements") {
            query.filter_supplements(target);
        } else if (type == "enhances") {
            query.filter_enhances(target);
        }
        dependents |= query;
    }
    std::vector<libdnf5::rpm::Package> sorted;
    for (const auto & dependent : dependents) {
        sorted.push_back(dependent);
    }
    sorted = normalize_candidates(std::move(sorted));
    for (const auto & dependent : sorted) {
        add_edge(node_id, add_package(dependent), {});
    }
}

void GraphBuilder::expand(const std::string & node_id) {
    const auto & node = graph.nodes.at(node_id);
    if (node.type == Node::Type::PACKAGE) {
        if (reverse) {
            expand_reverse(node_id, *node.package);
        } else {
            for (const auto & dependency : dependencies(*node.package)) {
                link_requirement(node_id, dependency);
            }
        }
    }
}

void prune_to_package_names(Graph & graph, const std::vector<std::string> & patterns) {
    // Invert the forward dependency edges so shortest paths can be reconstructed
    // from matching package nodes back to the selected roots.
    std::map<std::string, std::vector<std::string>> incoming;
    for (const auto & [source, outgoing] : graph.edges) {
        for (const auto & edge : outgoing) {
            incoming[edge.target].push_back(source);
        }
    }

    std::set<std::string> retained;
    std::set<std::pair<std::string, std::string>> retained_edges;
    std::vector<std::string> pending;
    // Match only RPM package names; structural nodes never act as pruning targets.
    for (const auto & [id, node] : graph.nodes) {
        if (node.type != Node::Type::PACKAGE ||
            std::none_of(patterns.begin(), patterns.end(), [&](const auto & pattern) {
                return fnmatch(pattern.c_str(), node.package->get_name().c_str(), 0) == 0;
            })) {
            continue;
        }
        retained.insert(id);
        pending.push_back(id);
    }

    // Reuse the minimum depths recorded during graph construction to reconstruct
    // all shortest paths backwards from every matching package.
    while (!pending.empty()) {
        const auto node_id = std::move(pending.back());
        pending.pop_back();
        const auto package_level = graph.nodes.at(node_id).type == Node::Type::PACKAGE ? 1 : 0;
        for (const auto & parent : incoming[node_id]) {
            // Retain edges that preserve the minimum root distance: zero for a
            // structural node, one when entering a package node.
            if (graph.min_depths.at(parent) + package_level != graph.min_depths.at(node_id)) {
                continue;
            }
            retained_edges.emplace(parent, node_id);
            if (retained.insert(parent).second) {
                pending.push_back(parent);
            }
        }
    }

    for (auto it = graph.nodes.begin(); it != graph.nodes.end();) {
        if (!retained.contains(it->first)) {
            graph.min_depths.erase(it->first);
            it = graph.nodes.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = graph.roots.begin(); it != graph.roots.end();) {
        if (!retained.contains(*it)) {
            it = graph.roots.erase(it);
        } else {
            ++it;
        }
    }
    for (auto edge_it = graph.edges.begin(); edge_it != graph.edges.end();) {
        if (!retained.contains(edge_it->first)) {
            edge_it = graph.edges.erase(edge_it);
            continue;
        }
        auto & outgoing = edge_it->second;
        outgoing.erase(
            std::remove_if(
                outgoing.begin(),
                outgoing.end(),
                [&](const Edge & edge) { return !retained_edges.contains({edge_it->first, edge.target}); }),
            outgoing.end());
        if (outgoing.empty()) {
            edge_it = graph.edges.erase(edge_it);
        } else {
            ++edge_it;
        }
    }
}

std::string printable_label(const Node & node) {
    switch (node.type) {
        case Node::Type::PACKAGE:
            return node.label;
        case Node::Type::CHOICE:
            return libdnf5::utils::sformat(_("choice: {}"), node.label);
        case Node::Type::CONDITIONAL:
            return libdnf5::utils::sformat(_("if {}:"), node.label);
        case Node::Type::ELSE_BRANCH:
            return _("else:");
        case Node::Type::REQUIREMENT:
            return libdnf5::utils::sformat(_("requirement: {}"), node.label);
        case Node::Type::UNSATISFIED:
            return libdnf5::utils::sformat(_("unsatisfied: {}"), node.label);
    }
    return {};
}

void print_tree_node(
    const std::string & node_id,
    const Graph & graph,
    const std::string & indent,
    bool last,
    std::set<std::string> & expanded,
    int level,
    bool root) {
    const auto & node = graph.nodes.at(node_id);
    const auto label = printable_label(node);
    const auto outgoing = graph.edges.find(node_id);
    // Leaf nodes have no subtree to defer or suppress.
    const bool has_children = outgoing != graph.edges.end() && !outgoing->second.empty();
    // A longer-path reference must not prevent expansion at a later shortest-path occurrence.
    const bool expanded_elsewhere = node.type == Node::Type::PACKAGE && has_children &&
                                    (level > graph.min_depths.at(node_id) || !expanded.insert(node_id).second);
    std::cout << (root ? "" : indent + (last ? "└─ " : "├─ "))
              << (expanded_elsewhere ? libdnf5::utils::sformat(_("{} [expanded elsewhere]"), label) : label)
              << std::endl;
    if (expanded_elsewhere) {
        return;
    }
    if (outgoing != graph.edges.end()) {
        for (size_t index = 0; index < outgoing->second.size(); ++index) {
            print_tree_node(
                outgoing->second[index].target,
                graph,
                root ? "" : indent + (last ? "   " : "│  "),
                index + 1 == outgoing->second.size(),
                expanded,
                level + (graph.nodes.at(outgoing->second[index].target).type == Node::Type::PACKAGE ? 1 : 0),
                false);
        }
    }
}

void print_tree(const Graph & graph) {
    bool first_root = true;
    std::set<std::string> expanded;
    for (const auto & root_id : graph.roots) {
        if (!first_root) {
            std::cout << std::endl;
        }
        first_root = false;
        print_tree_node(root_id, graph, "", true, expanded, 0, true);
    }
}

void print_json(const Graph & graph) {
    json_object * json_roots = json_object_new_array();
    json_object * json_nodes = json_object_new_array();
    json_object * json_edges = json_object_new_array();
    for (const auto & root : graph.roots) {
        json_object_array_add(json_roots, json_object_new_string(root.c_str()));
    }
    for (const auto & [id, node] : graph.nodes) {
        json_object * json_node = json_object_new_object();
        json_object_object_add(json_node, "id", json_object_new_string(id.c_str()));
        if (node.type == Node::Type::PACKAGE) {
            json_object_object_add(json_node, "type", json_object_new_string("package"));
            json_object_object_add(json_node, "name", json_object_new_string(node.package->get_name().c_str()));
            json_object_object_add(json_node, "epoch", json_object_new_string(node.package->get_epoch().c_str()));
            json_object_object_add(json_node, "version", json_object_new_string(node.package->get_version().c_str()));
            json_object_object_add(json_node, "release", json_object_new_string(node.package->get_release().c_str()));
            json_object_object_add(json_node, "arch", json_object_new_string(node.package->get_arch().c_str()));
        } else if (node.type == Node::Type::CHOICE) {
            json_object_object_add(json_node, "type", json_object_new_string("choice"));
            json_object_object_add(json_node, "capability", json_object_new_string(node.label.c_str()));
        } else if (node.type == Node::Type::CONDITIONAL) {
            json_object_object_add(json_node, "type", json_object_new_string("conditional"));
            json_object_object_add(json_node, "condition", json_object_new_string(node.label.c_str()));
        } else if (node.type == Node::Type::ELSE_BRANCH) {
            json_object_object_add(json_node, "type", json_object_new_string("else"));
            json_object_object_add(json_node, "condition", json_object_new_string(node.label.c_str()));
        } else if (node.type == Node::Type::REQUIREMENT) {
            json_object_object_add(json_node, "type", json_object_new_string("requirement"));
            json_object_object_add(json_node, "capability", json_object_new_string(node.label.c_str()));
        } else {
            json_object_object_add(json_node, "type", json_object_new_string("unsatisfied"));
            json_object_object_add(json_node, "capability", json_object_new_string(node.label.c_str()));
        }
        json_object_array_add(json_nodes, json_node);
    }
    for (const auto & [source, outgoing] : graph.edges) {
        for (const auto & edge : outgoing) {
            json_object * json_edge = json_object_new_object();
            json_object_object_add(json_edge, "source", json_object_new_string(source.c_str()));
            json_object_object_add(json_edge, "target", json_object_new_string(edge.target.c_str()));
            json_object * requirements = json_object_new_array();
            for (const auto & requirement : edge.requirements) {
                json_object_array_add(requirements, json_object_new_string(requirement.c_str()));
            }
            json_object_object_add(json_edge, "requirements", requirements);
            json_object_array_add(json_edges, json_edge);
        }
    }
    json_object * result = json_object_new_object();
    json_object_object_add(result, "roots", json_roots);
    json_object_object_add(result, "nodes", json_nodes);
    json_object_object_add(result, "edges", json_edges);
    std::cout << json_object_to_json_string_ext(result, JSON_C_TO_STRING_PRETTY) << std::endl;
    json_object_put(result);
}


}  // namespace dnf5
