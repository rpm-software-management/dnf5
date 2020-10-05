// TODO(dmach): keep refactoring and deliver something that works with the new code base
// the whole file is disabled via the SKIP macro because it doesn't compile with the new code
#ifdef SKIP

#ifndef LIBDNF_SWDB_RPMITEM_TEST_HPP
#define LIBDNF_SWDB_RPMITEM_TEST_HPP

#include "libdnf/transaction/Transformer.hpp"
#include "libdnf/utils/sqlite3/sqlite3.hpp"
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class TransformerMock : protected libdnf::transaction::Transformer {
public:
    TransformerMock();
    using libdnf::transaction::Transformer::Exception;
    using libdnf::transaction::Transformer::processGroupPersistor;
    using libdnf::transaction::Transformer::transformTrans;
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
    libdnf::utils::SQLite3 * swdb;
    libdnf::utils::SQLite3 * history;
};

#endif // LIBDNF_SWDB_RPMITEM_TEST_HPP

#endif
