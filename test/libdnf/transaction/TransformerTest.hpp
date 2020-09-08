#ifndef LIBDNF_SWDB_RPMITEM_TEST_HPP
#define LIBDNF_SWDB_RPMITEM_TEST_HPP

#include "libdnf/transaction/Transformer.hpp"
#include "libdnf/utils/sqlite3/sqlite3.hpp"
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class TransformerMock : protected libdnf::Transformer {
public:
    TransformerMock();
    using libdnf::Transformer::Exception;
    using libdnf::Transformer::processGroupPersistor;
    using libdnf::Transformer::transformTrans;
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
    std::shared_ptr< libdnf::utils::SQLite3 > swdb;
    std::shared_ptr< libdnf::utils::SQLite3 > history;
};

#endif // LIBDNF_SWDB_RPMITEM_TEST_HPP
