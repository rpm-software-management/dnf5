/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf5/rpm/package_query.hpp>
*/

// Create a package query.
libdnf5::rpm::PackageQuery packages(base);

// Filter the packages, the filters can be stacked one after another.
packages.filter_name("one");

// Iterate over the filtered packages in the query.
for (const auto & pkg : packages) {
    std::cout << pkg.get_name() << std::endl;
}
