// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#include "test_package_sack.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/rpm/package_sack.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <filesystem>
#include <set>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(RpmPackageSackTest);

using namespace libdnf5::rpm;

namespace {

// make constructor public so we can create Package instances in the tests
class TestPackage : public Package {
public:
    TestPackage(libdnf5::Base & base, PackageId id) : libdnf5::rpm::Package(base.get_weak_ptr(), id) {}
};

}  // namespace


void RpmPackageSackTest::setUp() {
    BaseTestCase::setUp();
    add_repo_solv("solv-24pkgs");

    pkgset = std::make_unique<PackageSet>(base);
    pkg0 = std::make_unique<TestPackage>(base, libdnf5::rpm::PackageId(0));
    pkgset->add(*pkg0);
}


void RpmPackageSackTest::test_set_user_excludes() {
    CPPUNIT_ASSERT(sack->get_user_excludes().size() == 0);
    sack->set_user_excludes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_excludes().contains(*pkg0) == true);
    CPPUNIT_ASSERT(sack->get_user_excludes().size() == 1);
    sack->clear_user_excludes();
    CPPUNIT_ASSERT(sack->get_user_excludes().size() == 0);
}


void RpmPackageSackTest::test_add_user_excludes() {
    CPPUNIT_ASSERT(sack->get_user_excludes().contains(*pkg0) == false);
    sack->add_user_excludes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_excludes().contains(*pkg0) == true);
    CPPUNIT_ASSERT(sack->get_user_excludes().size() == 1);

    // add the same package again
    sack->add_user_excludes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_excludes().size() == 1);
}


void RpmPackageSackTest::test_remove_user_excludes() {
    // remove package from empty excludes list does not fail
    CPPUNIT_ASSERT(sack->get_user_excludes().contains(*pkg0) == false);
    sack->remove_user_excludes(*pkgset);

    // remove existing package
    sack->set_user_excludes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_excludes().contains(*pkg0) == true);
    sack->remove_user_excludes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_excludes().contains(*pkg0) == false);
}


void RpmPackageSackTest::test_set_user_includes() {
    CPPUNIT_ASSERT(sack->get_user_includes().size() == 0);
    sack->set_user_includes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_includes().contains(*pkg0) == true);
    CPPUNIT_ASSERT(sack->get_user_includes().size() == 1);
    sack->clear_user_includes();
    CPPUNIT_ASSERT(sack->get_user_includes().size() == 0);
}


void RpmPackageSackTest::test_add_user_includes() {
    CPPUNIT_ASSERT(sack->get_user_includes().contains(*pkg0) == false);
    sack->add_user_includes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_includes().contains(*pkg0) == true);
    CPPUNIT_ASSERT(sack->get_user_includes().size() == 1);

    // add the same package again
    sack->add_user_includes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_includes().size() == 1);
}


void RpmPackageSackTest::test_remove_user_includes() {
    // remove package from empty includes list does not fail
    CPPUNIT_ASSERT(sack->get_user_includes().contains(*pkg0) == false);
    sack->remove_user_includes(*pkgset);

    // remove existing package
    sack->set_user_includes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_includes().contains(*pkg0) == true);
    sack->remove_user_includes(*pkgset);
    CPPUNIT_ASSERT(sack->get_user_includes().contains(*pkg0) == false);
}
