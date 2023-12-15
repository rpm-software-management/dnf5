/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf5/rpm/package_query.hpp>
*/

// Create a package query.
libdnf5::rpm::PackageQuery query(base);

// Filter the packages, the filters can be stacked one after another.
query.filter_name("one");

// Iterate over the filtered packages in the query.
for (const auto & pkg : query) {
    std::cout << pkg.get_nevra() << std::endl;
}
