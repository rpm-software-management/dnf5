#ifndef LIBDNF_SWDB_RPMITEM_TEST_HPP
#define LIBDNF_SWDB_RPMITEM_TEST_HPP

#include "libdnf/utils/sqlite3/Sqlite3.hpp"
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class RpmItemTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(RpmItemTest);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testCreateDuplicates);
    CPPUNIT_TEST(testGetTransactionItems);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testCreate();
    void testCreateDuplicates();
    void testGetTransactionItems();

private:
    std::shared_ptr< SQLite3 > conn;
};

#endif // LIBDNF_SWDB_RPMITEM_TEST_HPP
