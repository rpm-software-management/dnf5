# Create a package query.
query = libdnf5.rpm.PackageQuery(base)

# Filter the packages, the filters can be stacked one after another.
query.filter_name(["one"])

# Iterate over the filtered packages in the query.
for pkg in query:
    print(pkg.get_nevra())
