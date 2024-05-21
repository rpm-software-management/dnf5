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


#include "libdnf5/transaction/comps_group.hpp"

#include "db/comps_group.hpp"

#include "libdnf5/transaction/transaction.hpp"
#include "libdnf5/transaction/transaction_item.hpp"


namespace libdnf5::transaction {

class CompsGroup::Impl {
private:
    friend CompsGroup;

    std::string group_id;
    std::string name;
    std::string translated_name;
    libdnf5::comps::PackageType package_types;
    std::vector<CompsGroupPackage> packages;
};


CompsGroup::CompsGroup(const Transaction & trans)
    : TransactionItem::TransactionItem(trans),
      p_impl(std::make_unique<Impl>()) {}

CompsGroup::CompsGroup(const CompsGroup & src) : TransactionItem(src), p_impl(new Impl(*src.p_impl)) {}
CompsGroup::CompsGroup(CompsGroup && src) noexcept = default;

CompsGroup & CompsGroup::operator=(const CompsGroup & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
CompsGroup & CompsGroup::operator=(CompsGroup && src) noexcept = default;
CompsGroup::~CompsGroup() = default;

std::string CompsGroup::to_string() const {
    return get_group_id();
}

const std::string & CompsGroup::get_group_id() const noexcept {
    return p_impl->group_id;
}
void CompsGroup::set_group_id(const std::string & value) {
    p_impl->group_id = value;
}
const std::string & CompsGroup::get_name() const noexcept {
    return p_impl->name;
}
void CompsGroup::set_name(const std::string & value) {
    p_impl->name = value;
}
const std::string & CompsGroup::get_translated_name() const noexcept {
    return p_impl->translated_name;
}
void CompsGroup::set_translated_name(const std::string & value) {
    p_impl->translated_name = value;
}
libdnf5::comps::PackageType CompsGroup::get_package_types() const noexcept {
    return p_impl->package_types;
}
void CompsGroup::set_package_types(libdnf5::comps::PackageType value) {
    p_impl->package_types = value;
}
CompsGroupPackage & CompsGroup::new_package() {
    return p_impl->packages.emplace_back();
}
std::vector<CompsGroupPackage> & CompsGroup::get_packages() {
    return p_impl->packages;
}

class CompsGroupPackage::Impl {
private:
    friend CompsGroupPackage;
    int64_t id = 0;
    std::string name;
    bool installed = false;
    libdnf5::comps::PackageType package_type = libdnf5::comps::PackageType::DEFAULT;
};

CompsGroupPackage::CompsGroupPackage(const CompsGroupPackage & src) : p_impl(new Impl(*src.p_impl)) {}
CompsGroupPackage::CompsGroupPackage(CompsGroupPackage && src) noexcept = default;

CompsGroupPackage & CompsGroupPackage::operator=(const CompsGroupPackage & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
CompsGroupPackage & CompsGroupPackage::operator=(CompsGroupPackage && src) noexcept = default;
CompsGroupPackage::CompsGroupPackage() : p_impl(std::make_unique<Impl>()) {}
CompsGroupPackage::~CompsGroupPackage() = default;

int64_t CompsGroupPackage::get_id() const noexcept {
    return p_impl->id;
}
void CompsGroupPackage::set_id(int64_t value) {
    p_impl->id = value;
}
const std::string & CompsGroupPackage::get_name() const noexcept {
    return p_impl->name;
}
void CompsGroupPackage::set_name(const std::string & value) {
    p_impl->name = value;
}
bool CompsGroupPackage::get_installed() const noexcept {
    return p_impl->installed;
}
void CompsGroupPackage::set_installed(bool value) {
    p_impl->installed = value;
}
libdnf5::comps::PackageType CompsGroupPackage::get_package_type() const noexcept {
    return p_impl->package_type;
}
void CompsGroupPackage::set_package_type(libdnf5::comps::PackageType value) {
    p_impl->package_type = value;
}

/*
std::vector< TransactionItemPtr >
CompsGroup::getTransactionItemsByPattern(libdnf5::utils::SQLite3Ptr conn, const std::string &pattern)
{
    const char *sql = R"**(
        SELECT DISTINCT
            groupid
        FROM
            comps_group
        WHERE
            groupid LIKE ?
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
        auto groupid = query.get< std::string >("groupid");
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
