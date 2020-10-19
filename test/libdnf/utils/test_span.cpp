/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_span.hpp"

#include "libdnf/utils/span.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(UtilsSpanTest);


void UtilsSpanTest::setUp() {}


void UtilsSpanTest::tearDown() {}


void UtilsSpanTest::test_span() {
    int data[]{1, 2, 3, 4, 5, 6, 7, 8};
    libdnf::Span span1(data);

    CPPUNIT_ASSERT(!span1.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), span1.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8 * sizeof(int)), span1.size_bytes());

    CPPUNIT_ASSERT_EQUAL(1, span1.front());
    CPPUNIT_ASSERT_EQUAL(8, span1.back());
    CPPUNIT_ASSERT_EQUAL(5, span1[4]);
    CPPUNIT_ASSERT_EQUAL(static_cast<int *>(data), span1.data());

    CPPUNIT_ASSERT_EQUAL(1, *span1.begin());
    CPPUNIT_ASSERT_EQUAL(8, *(span1.end() - 1));
    CPPUNIT_ASSERT_EQUAL(8, *span1.rbegin());
    CPPUNIT_ASSERT_EQUAL(1, *(span1.rend() - 1));

    auto span2 = span1.first<6>();
    CPPUNIT_ASSERT_EQUAL(1, span2.front());
    CPPUNIT_ASSERT_EQUAL(6, span2.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), span2.size());

    auto dyn_span2 = span1.first(6);
    CPPUNIT_ASSERT_EQUAL(1, dyn_span2.front());
    CPPUNIT_ASSERT_EQUAL(6, dyn_span2.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), dyn_span2.size());

    auto span3 = span1.last<5>();
    CPPUNIT_ASSERT_EQUAL(4, span3.front());
    CPPUNIT_ASSERT_EQUAL(8, span3.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), span3.size());

    auto dyn_span3 = span1.last(5);
    CPPUNIT_ASSERT_EQUAL(4, dyn_span3.front());
    CPPUNIT_ASSERT_EQUAL(8, dyn_span3.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), dyn_span3.size());

    auto span4 = span1.subspan<4, 2>();
    CPPUNIT_ASSERT_EQUAL(5, span4.front());
    CPPUNIT_ASSERT_EQUAL(6, span4.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), span4.size());

    auto dyn_span4 = span1.subspan(4, 2);
    CPPUNIT_ASSERT_EQUAL(5, dyn_span4.front());
    CPPUNIT_ASSERT_EQUAL(6, dyn_span4.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), dyn_span4.size());

    std::array<int, 5> array_data = {11, 12, 13, 14, 15};
    libdnf::Span span5(array_data);
    CPPUNIT_ASSERT_EQUAL(11, span5.front());
    CPPUNIT_ASSERT_EQUAL(15, span5.back());
    CPPUNIT_ASSERT_EQUAL(12, span5[1]);

    libdnf::Span<int, 8> span_copy_assign;
    span_copy_assign = span1;

    CPPUNIT_ASSERT(!span_copy_assign.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), span_copy_assign.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8 * sizeof(int)), span_copy_assign.size_bytes());

    CPPUNIT_ASSERT_EQUAL(1, span_copy_assign.front());
    CPPUNIT_ASSERT_EQUAL(8, span_copy_assign.back());
    CPPUNIT_ASSERT_EQUAL(5, span_copy_assign[4]);
    CPPUNIT_ASSERT_EQUAL(static_cast<int *>(data), span_copy_assign.data());

    libdnf::Span<int, 8> span_move_assign;
    span_move_assign = libdnf::Span<int, 8>(data);

    CPPUNIT_ASSERT(!span_move_assign.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), span_move_assign.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8 * sizeof(int)), span_move_assign.size_bytes());

    CPPUNIT_ASSERT_EQUAL(1, span_move_assign.front());
    CPPUNIT_ASSERT_EQUAL(8, span_move_assign.back());
    CPPUNIT_ASSERT_EQUAL(5, span_move_assign[4]);
    CPPUNIT_ASSERT_EQUAL(static_cast<int *>(data), span_move_assign.data());
}

void UtilsSpanTest::test_dynamic_span() {
    int data[]{1, 2, 3, 4, 5, 6, 7, 8};
    libdnf::Span<int> span1(data);

    CPPUNIT_ASSERT(!span1.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), span1.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8 * sizeof(int)), span1.size_bytes());

    CPPUNIT_ASSERT_EQUAL(1, span1.front());
    CPPUNIT_ASSERT_EQUAL(8, span1.back());
    CPPUNIT_ASSERT_EQUAL(5, span1[4]);
    CPPUNIT_ASSERT_EQUAL(static_cast<int *>(data), span1.data());

    CPPUNIT_ASSERT_EQUAL(1, *span1.begin());
    CPPUNIT_ASSERT_EQUAL(8, *(span1.end() - 1));
    CPPUNIT_ASSERT_EQUAL(8, *span1.rbegin());
    CPPUNIT_ASSERT_EQUAL(1, *(span1.rend() - 1));

    auto span2 = span1.first<6>();
    CPPUNIT_ASSERT_EQUAL(1, span2.front());
    CPPUNIT_ASSERT_EQUAL(6, span2.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), span2.size());

    auto dyn_span2 = span1.first(6);
    CPPUNIT_ASSERT_EQUAL(1, dyn_span2.front());
    CPPUNIT_ASSERT_EQUAL(6, dyn_span2.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), dyn_span2.size());

    auto span3 = span1.last<5>();
    CPPUNIT_ASSERT_EQUAL(4, span3.front());
    CPPUNIT_ASSERT_EQUAL(8, span3.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), span3.size());

    auto dyn_span3 = span1.last(5);
    CPPUNIT_ASSERT_EQUAL(4, dyn_span3.front());
    CPPUNIT_ASSERT_EQUAL(8, dyn_span3.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), dyn_span3.size());

    auto span4 = span1.subspan<4, 2>();
    CPPUNIT_ASSERT_EQUAL(5, span4.front());
    CPPUNIT_ASSERT_EQUAL(6, span4.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), span4.size());

    auto dyn_span4 = span1.subspan(4, 2);
    CPPUNIT_ASSERT_EQUAL(5, dyn_span4.front());
    CPPUNIT_ASSERT_EQUAL(6, dyn_span4.back());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), dyn_span4.size());

    std::array<int, 5> array_data = {11, 12, 13, 14, 15};
    libdnf::Span<int> span5(array_data);
    CPPUNIT_ASSERT_EQUAL(11, span5.front());
    CPPUNIT_ASSERT_EQUAL(15, span5.back());
    CPPUNIT_ASSERT_EQUAL(12, span5[1]);

    libdnf::Span<int> span_copy_assign;
    span_copy_assign = span1;

    CPPUNIT_ASSERT(!span_copy_assign.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), span_copy_assign.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8 * sizeof(int)), span_copy_assign.size_bytes());

    CPPUNIT_ASSERT_EQUAL(1, span_copy_assign.front());
    CPPUNIT_ASSERT_EQUAL(8, span_copy_assign.back());
    CPPUNIT_ASSERT_EQUAL(5, span_copy_assign[4]);
    CPPUNIT_ASSERT_EQUAL(static_cast<int *>(data), span_copy_assign.data());

    libdnf::Span<int> span_move_assign;
    span_move_assign = libdnf::Span<int>(data);

    CPPUNIT_ASSERT(!span_move_assign.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), span_move_assign.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8 * sizeof(int)), span_move_assign.size_bytes());

    CPPUNIT_ASSERT_EQUAL(1, span_move_assign.front());
    CPPUNIT_ASSERT_EQUAL(8, span_move_assign.back());
    CPPUNIT_ASSERT_EQUAL(5, span_move_assign[4]);
    CPPUNIT_ASSERT_EQUAL(static_cast<int *>(data), span_move_assign.data());
}

void UtilsSpanTest::test_span_iterator() {
    std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int> reverse_data(data.rbegin(), data.rend());

    libdnf::Span<int, 8> span(data.data());
    std::vector<int> result;
    for (auto value : span) {
        result.push_back(value);
    }
    CPPUNIT_ASSERT(data == result);

    result.clear();
    for (auto it = span.rbegin(); it != span.rend(); ++it) {
        result.push_back(*it);
    }
    CPPUNIT_ASSERT(reverse_data == result);

    libdnf::Span<int> dyn_span(data.data(), data.size());
    result.clear();
    for (auto value : dyn_span) {
        result.push_back(value);
    }
    CPPUNIT_ASSERT(data == result);

    result.clear();
    for (auto it = dyn_span.rbegin(); it != dyn_span.rend(); ++it) {
        result.push_back(*it);
    }
    CPPUNIT_ASSERT(reverse_data == result);
}
