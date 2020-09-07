#ifndef LIBDNF_SWDB_TRANSACTION_TEST_HPP
#define LIBDNF_SWDB_TRANSACTION_TEST_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "libdnf/utils/sqlite3/Sqlite3.hpp"

class TransactionTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TransactionTest);
    CPPUNIT_TEST(testInsert);
    CPPUNIT_TEST(testInsertWithSpecifiedId);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testComparison);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testInsert();
    void testInsertWithSpecifiedId();
    void testUpdate();
    void testComparison();

private:
    std::shared_ptr< SQLite3 > conn;
};

#endif // LIBDNF_SWDB_TRANSACTION_TEST_HPP
