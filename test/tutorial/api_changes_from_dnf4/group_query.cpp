/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf5/comps/group_query.hpp>
*/

// Create a group query.
libdnf5::comps::GroupQuery groups(base);

// Filter the groups, the filters can be stacked one after another.
groups.filter_groupid("group-id");

// Iterate over the filtered groups in the query.
for (const auto & group : groups) {
    std::cout << group.get_name() << std::endl;
}
