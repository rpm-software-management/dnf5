#ifndef LIBDNF_SWDB_MERGEDTRANSACTION_TEST_HPP
#define LIBDNF_SWDB_MERGEDTRANSACTION_TEST_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "libdnf/utils/sqlite3/sqlite3.hpp"

class MergedTransactionTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(MergedTransactionTest);
//     CPPUNIT_TEST(testMerge);
//     CPPUNIT_TEST(testMergeEraseInstallReinstall);
//     CPPUNIT_TEST(testMergeEraseInstallDowngrade);
//     CPPUNIT_TEST(testMergeEraseInstallUpgrade);
//     CPPUNIT_TEST(testMergeReinstallAny);
//     CPPUNIT_TEST(testMergeInstallErase);
//     CPPUNIT_TEST(testMergeInstallAlter);
//     CPPUNIT_TEST(testMergeAlterReinstall);
//     CPPUNIT_TEST(testMergeAlterErase);
//     CPPUNIT_TEST(testMergeAlterAlter);
    CPPUNIT_TEST(test_add_remove_installed);
    CPPUNIT_TEST(test_add_remove_removed);
    CPPUNIT_TEST(test_add_install_installed);
    CPPUNIT_TEST(test_add_install_removed);
    CPPUNIT_TEST(test_add_obsoleted_installed);
    CPPUNIT_TEST(test_add_obsoleted_obsoleted);

    CPPUNIT_TEST(test_downgrade);
    CPPUNIT_TEST(test_install_downgrade);

    CPPUNIT_TEST(test_multilib_identity);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testMerge();
    void testMergeEraseInstallReinstall();
    void testMergeEraseInstallDowngrade();
    void testMergeEraseInstallUpgrade();
    void testMergeReinstallAny();
    void testMergeInstallErase();
    void testMergeInstallAlter();
    void testMergeAlterReinstall();
    void testMergeAlterErase();
    void testMergeAlterAlter();
    // BEGIN: tests ported from DNF unit tests
    void test_add_remove_installed();
    void test_add_remove_removed();
    void test_add_install_installed();
    void test_add_install_removed();
    void test_add_obsoleted_installed();
    void test_add_obsoleted_obsoleted();
    // END: tests ported from DNF unit tests

    void test_downgrade();
    void test_install_downgrade();

    void test_multilib_identity();
private:
    std::shared_ptr< libdnf::utils::SQLite3 > conn;
};

#endif // LIBDNF_SWDB_MERGEDTRANSACTION_TEST_HPP
