// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "test_query.hpp"

#include <libdnf5/common/sack/query.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(QueryTest);

namespace {

class QueryItem {
public:
    bool enabled;
    int id;
    std::string name;

    bool operator==(const QueryItem & other) const noexcept {
        return enabled == other.enabled && id == other.id && name == other.name;
    }
    bool operator<(const QueryItem & other) const noexcept { return id < other.id; }
};

// Simple query implementation based on libdnf5::sack::Query that works on the QueryItem instances and implements 3 filters.
class TestQuery : public libdnf5::sack::Query<QueryItem> {
public:
    using Query<QueryItem>::Query;

    TestQuery & filter_enabled(bool enabled);
    TestQuery & filter_id(int64_t id, libdnf5::sack::QueryCmp cmp);
    TestQuery & filter_name(const std::string & name, libdnf5::sack::QueryCmp cmp);

private:
    struct F {
        static bool enabled(const QueryItem & obj) { return obj.enabled; }
        static int64_t id(const QueryItem & obj) { return obj.id; }
        static std::string name(const QueryItem & obj) { return obj.name; }
    };
};

TestQuery & TestQuery::filter_enabled(bool enabled) {
    filter(F::enabled, enabled, libdnf5::sack::QueryCmp::EQ);
    return *this;
}

TestQuery & TestQuery::filter_id(int64_t id, libdnf5::sack::QueryCmp cmp) {
    filter(F::id, id, cmp);
    return *this;
}

TestQuery & TestQuery::filter_name(const std::string & name, libdnf5::sack::QueryCmp cmp) {
    filter(F::name, name, cmp);
    return *this;
}

}  // namespace


void QueryTest::setUp() {}


void QueryTest::tearDown() {}


void QueryTest::test_query_basics() {
    TestQuery q(libdnf5::Set<QueryItem>{{true, 4, "item4"}, {false, 6, "item6"}, {true, 10, "item10"}});
    CPPUNIT_ASSERT_EQUAL(q.size(), static_cast<size_t>(3));

    auto q1 = q;
    CPPUNIT_ASSERT_EQUAL(q1.size(), static_cast<size_t>(3));

    // test filter_enabled()
    q1.filter_enabled(true);
    CPPUNIT_ASSERT_EQUAL(q1.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT((q1 == libdnf5::Set<QueryItem>{{true, 4, "item4"}, {true, 10, "item10"}}));

    // test filter_name()
    q1.filter_name("item10", libdnf5::sack::QueryCmp::EQ);
    CPPUNIT_ASSERT_EQUAL(q1.size(), static_cast<size_t>(1));
    CPPUNIT_ASSERT((q1 == libdnf5::Set<QueryItem>{{true, 10, "item10"}}));

    // test filter_id()
    q1 = q;
    q1.filter_id(6, libdnf5::sack::QueryCmp::GTE);
    CPPUNIT_ASSERT_EQUAL(q1.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT((q1 == libdnf5::Set<QueryItem>{{false, 6, "item6"}, {true, 10, "item10"}}));

    // test chaining of filter calling
    q1 = q;
    q1.filter_name("item?", libdnf5::sack::QueryCmp::GLOB).filter_enabled(false);
    CPPUNIT_ASSERT_EQUAL(q1.size(), static_cast<size_t>(1));
    CPPUNIT_ASSERT((q1 == libdnf5::Set<QueryItem>{{false, 6, "item6"}}));
}
