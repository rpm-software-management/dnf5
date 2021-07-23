/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_tutorial.hpp"

#include "libdnf/base/base.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(TutorialTest);


void TutorialTest::setUp() {
    temp = std::make_unique<libdnf::utils::TempDir>("libdnf_unittest_");
    installroot = temp->get_path().native() + "/installroot";
}


void TutorialTest::tearDown() {
}


void TutorialTest::test_create_base() {
    #include "session/create_base.cpp"
}


void TutorialTest::test_load_repo() {
    #include "session/create_base.cpp"
    #include "repo/load_repo.cpp"
}
