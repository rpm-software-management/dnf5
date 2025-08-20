# Create a group query.
groups = libdnf5.comps.GroupQuery(base)

# Filter the groups, the filters can be stacked one after another.
groups.filter_groupid("group-id")

# Iterate over the filtered groups in the query.
for group in groups:
    print(group.get_name())
