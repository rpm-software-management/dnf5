/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "comps_group_package.hpp"

#include "pkg_name.hpp"

#include "libdnf/comps/group/package.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <algorithm>
#include <memory>


namespace libdnf::transaction {


static constexpr const char * SQL_COMPS_GROUP_PACKAGE_SELECT = R"**(
    SELECT
        "cgp"."id",
        "pkg_name"."name",
        "cgp"."installed",
        "cgp"."pkg_type"
    FROM
        "comps_group_package" "cgp"
    LEFT JOIN "pkg_name" ON "cgp"."name_id" = "pkg_name"."id"
    WHERE
        "cgp"."group_id" = ?
    ORDER BY
        "cgp"."id"
)**";


static std::unique_ptr<libdnf::utils::SQLite3::Query> comps_group_package_select_new_query(
    libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Query>(conn, SQL_COMPS_GROUP_PACKAGE_SELECT);
    return query;
}


void CompsGroupPackageDbUtils::comps_group_packages_select(libdnf::utils::SQLite3 & conn, CompsGroup & group) {
    auto query = comps_group_package_select_new_query(conn);
    query->bindv(group.get_item_id());

    while (query->step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        auto & pkg = group.new_package();
        pkg.set_id(query->get<int64_t>("id"));
        pkg.set_name(query->get<std::string>("name"));
        pkg.set_installed(query->get<bool>("installed"));
        pkg.set_package_type(static_cast<comps::PackageType>(query->get<int>("pkg_type")));
    }
}


static constexpr const char * SQL_COMPS_GROUP_PACKAGE_INSERT = R"**(
    INSERT INTO
        "comps_group_package" (
            "group_id",
            "name_id",
            "installed",
            "pkg_type"
        )
    VALUES
        (?, (SELECT "id" FROM "pkg_name" WHERE "name" = ?), ?, ?)
)**";


static std::unique_ptr<libdnf::utils::SQLite3::Statement> comps_group_package_insert_new_query(
    libdnf::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf::utils::SQLite3::Statement>(conn, SQL_COMPS_GROUP_PACKAGE_INSERT);
    return query;
}


void CompsGroupPackageDbUtils::comps_group_packages_insert(libdnf::utils::SQLite3 & conn, CompsGroup & group) {
    auto query_pkg_name_insert_if_not_exists = pkg_name_insert_if_not_exists_new_query(conn);
    auto query = comps_group_package_insert_new_query(conn);
    for (auto & pkg : group.get_packages()) {
        // insert package name into 'pkg_name' table if not exists
        pkg_name_insert_if_not_exists(*query_pkg_name_insert_if_not_exists, pkg.get_name());
        query->bindv(
            group.get_item_id(), pkg.get_name(), pkg.get_installed(), static_cast<int>(pkg.get_package_type()));
        query->step();
        pkg.set_id(query->last_insert_rowid());
        query->reset();
    }
}


}  // namespace libdnf::transaction
