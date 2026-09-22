// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_PLUGINS_DEPTREE_PLUGIN_DEPTREE_GRAPH_HPP
#define DNF5_PLUGINS_DEPTREE_PLUGIN_DEPTREE_GRAPH_HPP

#include <json-c/json.h>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/reldep.hpp>

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace dnf5 {

// A directed graph edge.
struct Edge {
    std::string target;  // Stable ID of the target node.
    // Capabilities represented by this edge; several can share one target.
    std::vector<std::string> requirements;
};

// A package node or a display-only node preserving dependency expression structure.
struct Node {
    // PACKAGE represents an RPM.
    // CHOICE represents alternative provider candidates.
    // CONDITIONAL and ELSE_BRANCH preserve rich-dependency control flow.
    // REQUIREMENT exposes an individual capability when --show-requires is used.
    // UNSATISFIED represents a dependency for which no provider is available.
    enum class Type { PACKAGE, CHOICE, CONDITIONAL, ELSE_BRANCH, REQUIREMENT, UNSATISFIED };
    Type type;
    std::string label;  // Text used by the tree and JSON renderers.
    // Present only for package nodes, which can be expanded into dependencies.
    std::optional<libdnf5::rpm::Package> package;
};

// The complete dependency graph produced by GraphBuilder.
struct Graph {
    // Graph nodes keyed by stable IDs.
    std::map<std::string, Node> nodes;
    // Outgoing edges keyed by source node ID; insertion order determines text and JSON output order.
    std::map<std::string, std::vector<Edge>> edges;
    // Root node IDs, deduplicated by NEVRA.
    std::set<std::string> roots;
};

class GraphBuilder {
public:
    GraphBuilder(
        libdnf5::Base & base,
        const std::set<std::string> & types,
        const std::set<std::string> & arches,
        bool reverse,
        bool show_requires,
        bool show_duplicates,
        int depth);

    // Builds a dependency graph rooted at the supplied packages.
    Graph build(const libdnf5::rpm::PackageQuery & root_packages);

private:
    libdnf5::Base & base;
    // All packages in the selected source after applying excludes.
    libdnf5::rpm::PackageQuery available;
    // Enabled dependency relation kinds.
    const std::set<std::string> & types;
    // Exact architecture filter, empty when all architectures are included.
    const std::set<std::string> & arches;
    // Cached latest packages query for reverse traversal.
    libdnf5::rpm::PackageQuery latest_available;
    bool reverse;
    bool show_requires;
    bool show_duplicates;
    int max_depth;
    Graph graph;
    // Nodes already traversed, preventing repeated expansion.
    std::set<std::string> already_expanded;

    // Adds a package node and returns its stable ID.
    std::string add_package(const libdnf5::rpm::Package & package);

    // Adds a display-only node and returns its stable ID.
    // identity distinguishes nodes with the same display label but different expressions.
    std::string add_node(Node::Type type, const std::string & label, const std::string & identity = {});

    // Links two nodes, collecting requirement labels for a shared target.
    void add_edge(const std::string & source, const std::string & target, std::string requirement);

    // Sorts and deduplicates provider candidates for stable output.
    std::vector<libdnf5::rpm::Package> normalize_candidates(std::vector<libdnf5::rpm::Package> candidates) const;

    // Finds packages that satisfy a dependency.
    std::vector<libdnf5::rpm::Package> providers(const libdnf5::rpm::Reldep & dependency, bool latest = true);
    // Keeps the latest EVR of each candidate name and architecture.
    std::vector<libdnf5::rpm::Package> latest_candidates(std::vector<libdnf5::rpm::Package> candidates) const;
    // Connects a dependency to its provider candidates, adding a choice node when needed.
    void link_candidates(
        const std::string & parent,
        const std::string & requirement,
        const std::vector<libdnf5::rpm::Package> & candidates,
        const std::string & choice_label = {});

    // Adds graph nodes and edges for a dependency expression.
    void link_requirement(const std::string & parent, const libdnf5::rpm::Reldep & dependency);

    // Returns the enabled dependency relations of a package.
    libdnf5::rpm::ReldepList dependencies(const libdnf5::rpm::Package & package) const;

    // Adds reverse dependencies for a package node.
    void expand_reverse(const std::string & node_id, const libdnf5::rpm::Package & package);

    // Expands one package node unless it was already visited or exceeds the depth limit.
    void expand(std::string node_id, int level);
};

void print_tree(const Graph & graph);
void print_json(const Graph & graph);

}  // namespace dnf5

#endif  // DNF5_PLUGINS_DEPTREE_PLUGIN_DEPTREE_GRAPH_HPP
