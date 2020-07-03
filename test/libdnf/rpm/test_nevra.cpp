/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "test_nevra.hpp"

#include "libdnf/rpm/nevra.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(NevraTest);


void NevraTest::setUp() {}


void NevraTest::tearDown() {}


void NevraTest::test_nevra() {
    libdnf::rpm::Nevra nevra;
    {
        CPPUNIT_ASSERT(nevra.parse("four-of-fish-8:3.6.9-11.fc100.x86_64", libdnf::rpm::Nevra::Form::NEVRA));
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "8");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");
    }

    {
        CPPUNIT_ASSERT(nevra.parse("four-of-fish-3.6.9-11.fc100.x86_64", libdnf::rpm::Nevra::Form::NEVRA));
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");
    }

    {
        CPPUNIT_ASSERT(nevra.parse("four-of-fish-8:3.6.9", libdnf::rpm::Nevra::Form::NEV));
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "8");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "");
        CPPUNIT_ASSERT(nevra.get_arch() == "");
    }

    {
        CPPUNIT_ASSERT(nevra.parse("four-of-fish-3.6.9.i686", libdnf::rpm::Nevra::Form::NA));
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish-3.6.9");
        CPPUNIT_ASSERT(nevra.get_epoch() == "");
        CPPUNIT_ASSERT(nevra.get_version() == "");
        CPPUNIT_ASSERT(nevra.get_release() == "");
        CPPUNIT_ASSERT(nevra.get_arch() == "i686");
    }

    {
        CPPUNIT_ASSERT(nevra.parse("four-of-fish-3.6.9.i686", libdnf::rpm::Nevra::Form::NEV));
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9.i686");
        CPPUNIT_ASSERT(nevra.get_release() == "");
        CPPUNIT_ASSERT(nevra.get_arch() == "");
    }

    // When parsing fails return false
    {
        CPPUNIT_ASSERT(!nevra.parse("four-of-fish", libdnf::rpm::Nevra::Form::NA));
    }

    // When parsing fails return false - not allowed characters '()'
    {
        CPPUNIT_ASSERT(!nevra.parse("four-of(fish.i686)", libdnf::rpm::Nevra::Form::NA));
    }

    // Test for correct parsin with glob in epoch
    {
        CPPUNIT_ASSERT(nevra.parse("four-of-fish-[01]:3.6.9-11.fc100.x86_64", libdnf::rpm::Nevra::Form::NEVRA));
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "[01]");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");
    }

    // Test for correct parsin with glob in epoch
    {
        CPPUNIT_ASSERT(nevra.parse("four-of-fish-?:3.6.9-11.fc100.x86_64", libdnf::rpm::Nevra::Form::NEVRA));
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "?");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");
    }
}
