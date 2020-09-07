#ifndef LIBDNF_SWDB_COMPSENVIRONMENTITEM_TEST_HPP
#define LIBDNF_SWDB_COMPSENVIRONMENTITEM_TEST_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "libdnf/utils/sqlite3/Sqlite3.hpp"

class CompsGroupItemTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(CompsGroupItemTest);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testGetTransactionItems);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testCreate();
    void testGetTransactionItems();

private:
    std::shared_ptr< SQLite3 > conn;
};

#endif // LIBDNF_SWDB_COMPSENVIRONMENTITEM_TEST_HPP
