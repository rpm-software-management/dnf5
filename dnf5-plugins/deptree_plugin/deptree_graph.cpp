// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "deptree_graph.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/format.hpp>

#include <algorithm>
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
      latest_available(base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES),
      reverse(reverse),
      show_requires(show_requires),
      show_duplicates(show_duplicates),
      max_depth(depth) {
    if (!arches.empty()) {
        const std::vector<std::string> architecture_patterns(arches.begin(), arches.end());
        available.filter_arch(architecture_patterns);
        latest_available.filter_arch(architecture_patterns);
    }
    latest_available.filter_latest_evr();
}

Graph GraphBuilder::build(const libdnf5::rpm::PackageQuery & root_packages) {
    for (const auto & root : root_packages) {
        // The NEVRA-derived node ID deduplicates roots resolved from different specs.
        graph.roots.insert(add_package(root));
    }
    for (const auto & root : graph.roots) {
        expand(root, 0);
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
        libdnf5::rpm::PackageQuery query(show_duplicates ? available : latest_available);
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

void GraphBuilder::expand(std::string node_id, int level) {
    if (max_depth >= 0 && level >= max_depth) {
        return;
    }
    // TODO: Revisit packages reached later through a shallower path when depth-limited traversal needs it.
    // For now, keep first-encounter traversal to avoid path-dependent tree rendering complexity.
    // Prevent cycles and repeated subtrees.
    if (!already_expanded.insert(node_id).second) {
        return;
    }
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
    // Copy outgoing edges because recursion may append to an edge list and
    // invalidate iterators into its vector.
    const auto outgoing = graph.edges[node_id];
    for (const auto & edge : outgoing) {
        // Only bump the level for PACKAGE type nodes
        const auto next_level = level + (graph.nodes.at(edge.target).type == Node::Type::PACKAGE ? 1 : 0);
        expand(edge.target, next_level);
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
    std::set<std::string> & seen,
    bool root) {
    const auto label = printable_label(graph.nodes.at(node_id));
    const auto outgoing = graph.edges.find(node_id);
    // suppress [already shown] for nodes that do not have outgoing edges
    const bool has_children = outgoing != graph.edges.end() && !outgoing->second.empty();
    const bool repeated =
        graph.nodes.at(node_id).type == Node::Type::PACKAGE && has_children && !seen.insert(node_id).second;
    std::cout << (root ? "" : indent + (last ? "└─ " : "├─ "))
              << (repeated ? libdnf5::utils::sformat(_("{} [already shown]"), label) : label) << std::endl;
    if (repeated) {
        return;
    }
    if (outgoing != graph.edges.end()) {
        for (size_t index = 0; index < outgoing->second.size(); ++index) {
            print_tree_node(
                outgoing->second[index].target,
                graph,
                root ? "" : indent + (last ? "   " : "│  "),
                index + 1 == outgoing->second.size(),
                seen,
                false);
        }
    }
}

void print_tree(const Graph & graph) {
    bool first_root = true;
    for (const auto & root_id : graph.roots) {
        if (!first_root) {
            std::cout << std::endl;
        }
        first_root = false;
        std::set<std::string> seen;
        print_tree_node(root_id, graph, "", true, seen, true);
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
