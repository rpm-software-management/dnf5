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

// TODO(dmach): keep refactoring and deliver something that works with the new code base
// the whole file is disabled via the SKIP macro because it doesn't compile with the new code
#ifdef SKIP

#ifndef LIBDNF5_SWDB_RPMITEM_TEST_HPP
#define LIBDNF5_SWDB_RPMITEM_TEST_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/transaction/Transformer.hpp>
#include <libdnf5/utils/sqlite3/sqlite3.hpp>

class TransformerMock : protected libdnf5::transaction::Transformer {
public:
    TransformerMock();
    using libdnf5::transaction::Transformer::Exception;
    using libdnf5::transaction::Transformer::processGroupPersistor;
    using libdnf5::transaction::Transformer::transformTrans;
};

class TransformerTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TransformerTest);
    CPPUNIT_TEST(testGroupTransformation);
    CPPUNIT_TEST(testTransformTrans);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testTransformTrans();
    void testGroupTransformation();

protected:
    TransformerMock transformer;
    libdnf5::utils::SQLite3 * swdb;
    libdnf5::utils::SQLite3 * history;
};

#endif  // LIBDNF5_SWDB_RPMITEM_TEST_HPP

#endif
