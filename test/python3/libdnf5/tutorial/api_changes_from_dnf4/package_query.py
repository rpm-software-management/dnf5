# Create a package query.
packages = libdnf5.rpm.PackageQuery(base)

# Filter the packages, the filters can be stacked one after another.
packages.filter_name(["one"])

# Iterate over the filtered packages in the query.
for pkg in packages:
    print(pkg.get_name())
