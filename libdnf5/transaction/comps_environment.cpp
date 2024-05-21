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


#include "libdnf5/transaction/comps_environment.hpp"

#include "db/comps_environment.hpp"

#include "libdnf5/transaction/transaction.hpp"
#include "libdnf5/transaction/transaction_item.hpp"


namespace libdnf5::transaction {


class CompsEnvironment::Impl {
public:
private:
    friend CompsEnvironment;

    std::string environment_id;
    std::string name;
    std::string translated_name;
    libdnf5::comps::PackageType package_types = libdnf5::comps::PackageType::DEFAULT;
    std::vector<CompsEnvironmentGroup> groups;
};

CompsEnvironment::CompsEnvironment(const Transaction & trans)
    : TransactionItem::TransactionItem(trans),
      p_impl(std::make_unique<Impl>()) {}

CompsEnvironment::CompsEnvironment(const CompsEnvironment & src)
    : TransactionItem(src),
      p_impl(new Impl(*src.p_impl)) {}
CompsEnvironment::CompsEnvironment(CompsEnvironment && src) noexcept = default;

CompsEnvironment & CompsEnvironment::operator=(const CompsEnvironment & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
CompsEnvironment & CompsEnvironment::operator=(CompsEnvironment && src) noexcept = default;
CompsEnvironment::~CompsEnvironment() = default;

std::string CompsEnvironment::to_string() const {
    return get_environment_id();
}

const std::string & CompsEnvironment::get_environment_id() const noexcept {
    return p_impl->environment_id;
}
void CompsEnvironment::set_environment_id(const std::string & value) {
    p_impl->environment_id = value;
}
const std::string & CompsEnvironment::get_name() const noexcept {
    return p_impl->name;
}
void CompsEnvironment::set_name(const std::string & value) {
    p_impl->name = value;
}
const std::string & CompsEnvironment::get_translated_name() const noexcept {
    return p_impl->translated_name;
}
void CompsEnvironment::set_translated_name(const std::string & value) {
    p_impl->translated_name = value;
}
libdnf5::comps::PackageType CompsEnvironment::get_package_types() const noexcept {
    return p_impl->package_types;
}
void CompsEnvironment::set_package_types(libdnf5::comps::PackageType value) {
    p_impl->package_types = value;
}
CompsEnvironmentGroup & CompsEnvironment::new_group() {
    return p_impl->groups.emplace_back();
}
std::vector<CompsEnvironmentGroup> & CompsEnvironment::get_groups() {
    return p_impl->groups;
}

class CompsEnvironmentGroup::Impl {
private:
    friend CompsEnvironmentGroup;
    int64_t id = 0;
    std::string group_id;
    bool installed = false;
    libdnf5::comps::PackageType group_type;
};


CompsEnvironmentGroup::CompsEnvironmentGroup(const CompsEnvironmentGroup & src) : p_impl(new Impl(*src.p_impl)) {}
CompsEnvironmentGroup::CompsEnvironmentGroup(CompsEnvironmentGroup && src) noexcept = default;

CompsEnvironmentGroup & CompsEnvironmentGroup::operator=(const CompsEnvironmentGroup & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
CompsEnvironmentGroup & CompsEnvironmentGroup::operator=(CompsEnvironmentGroup && src) noexcept = default;
CompsEnvironmentGroup::CompsEnvironmentGroup() : p_impl(std::make_unique<Impl>()) {}
CompsEnvironmentGroup::~CompsEnvironmentGroup() = default;
int64_t CompsEnvironmentGroup::get_id() const noexcept {
    return p_impl->id;
}
void CompsEnvironmentGroup::set_id(int64_t value) {
    p_impl->id = value;
}
const std::string & CompsEnvironmentGroup::get_group_id() const noexcept {
    return p_impl->group_id;
}
void CompsEnvironmentGroup::set_group_id(const std::string & value) {
    p_impl->group_id = value;
}
bool CompsEnvironmentGroup::get_installed() const noexcept {
    return p_impl->installed;
}
void CompsEnvironmentGroup::set_installed(bool value) {
    p_impl->installed = value;
}
libdnf5::comps::PackageType CompsEnvironmentGroup::get_group_type() const noexcept {
    return p_impl->group_type;
}
void CompsEnvironmentGroup::set_group_type(libdnf5::comps::PackageType value) {
    p_impl->group_type = value;
}


/*
std::vector< TransactionItemPtr >
CompsEnvironmentItem::getTransactionItemsByPattern(libdnf5::utils::SQLite3Ptr conn, const std::string &pattern)
{
    string sql = R"**(
            SELECT DISTINCT
                environmentid
            FROM
                comps_environment
            WHERE
                environmentid LIKE ?
                OR name LIKE ?
                OR translated_name LIKE ?
        )**";

    std::vector< TransactionItemPtr > result;

    // HACK: create a private connection to avoid undefined behavior
    // after forking process in Anaconda
    libdnf5::utils::SQLite3 privateConn(conn->get_path());
    libdnf5::utils::SQLite3::Query query(privateConn, sql);
    std::string pattern_sql = pattern;
    std::replace(pattern_sql.begin(), pattern_sql.end(), '*', '%');
    query.bindv(pattern, pattern, pattern);
    while (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        auto groupid = query.get< std::string >("environmentid");
        auto trans_item = getTransactionItem(conn, groupid);
        if (!trans_item) {
            continue;
        }
        result.push_back(trans_item);
    }
    return result;
}
*/


}  // namespace libdnf5::transaction
