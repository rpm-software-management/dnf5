#ifndef LIBDNF_SWDB_WORKFLOW_TEST_HPP
#define LIBDNF_SWDB_WORKFLOW_TEST_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "libdnf/utils/sqlite3/Sqlite3.hpp"

class WorkflowTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(WorkflowTest);
    CPPUNIT_TEST(testDefaultWorkflow);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testDefaultWorkflow();

private:
    std::shared_ptr< SQLite3 > conn;
};

#endif // LIBDNF_SWDB_WORKFLOW_TEST_HPP
