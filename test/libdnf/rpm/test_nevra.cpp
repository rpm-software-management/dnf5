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
    {
        auto nevras =
            libdnf::rpm::Nevra::parse("four-of-fish-8:3.6.9-11.fc100.x86_64", {libdnf::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL(1ul, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "8");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");
        // test that to_nevra_string() and to_full_nevra_string() template functions work
        CPPUNIT_ASSERT_EQUAL(std::string("four-of-fish-8:3.6.9-11.fc100.x86_64"), to_nevra_string(nevra));
        CPPUNIT_ASSERT_EQUAL(std::string("four-of-fish-8:3.6.9-11.fc100.x86_64"), to_full_nevra_string(nevra));
    }

    // cannot parse due to ':' in name
    {
        auto nevras =
            libdnf::rpm::Nevra::parse("four-of-f:ish-3.6.9-11.fc100.x86_64", {libdnf::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL(0ul, nevras.size());
    }

    // cannot parse due to ':' presence twice
    {
        CPPUNIT_ASSERT_THROW(
            libdnf::rpm::Nevra::parse("four-of-fish-8:9:3.6.9-11.fc100.x86_64", {libdnf::rpm::Nevra::Form::NEVRA}),
            libdnf::rpm::Nevra::IncorrectNevraString);
    }


    {
        auto nevras =
            libdnf::rpm::Nevra::parse("four-of-fish-3.6.9-11.fc100.x86_64", {libdnf::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL(1ul, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");
    }

    {
        auto nevras = libdnf::rpm::Nevra::parse("four-of-fish-8:3.6.9", {libdnf::rpm::Nevra::Form::NEV});
        CPPUNIT_ASSERT_EQUAL(1ul, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "8");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "");
        CPPUNIT_ASSERT(nevra.get_arch() == "");
    }

    {
        auto nevras = libdnf::rpm::Nevra::parse("fish-8:3.6.9", {libdnf::rpm::Nevra::Form::NEV});
        CPPUNIT_ASSERT_EQUAL(1ul, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT_EQUAL(std::string("fish"), nevra.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("8"), nevra.get_epoch());
        CPPUNIT_ASSERT_EQUAL(std::string("3.6.9"), nevra.get_version());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_release());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_arch());
    }

    {
        auto nevras = libdnf::rpm::Nevra::parse("fish-3.6.9", {libdnf::rpm::Nevra::Form::NEV});
        CPPUNIT_ASSERT_EQUAL(1ul, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT_EQUAL(std::string("fish"), nevra.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_epoch());
        CPPUNIT_ASSERT_EQUAL(std::string("3.6.9"), nevra.get_version());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_release());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_arch());
    }


    {
        auto nevras = libdnf::rpm::Nevra::parse("four-of-fish-3.6.9.i686", {libdnf::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL(1ul, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish-3.6.9");
        CPPUNIT_ASSERT(nevra.get_epoch() == "");
        CPPUNIT_ASSERT(nevra.get_version() == "");
        CPPUNIT_ASSERT(nevra.get_release() == "");
        CPPUNIT_ASSERT(nevra.get_arch() == "i686");
    }

    {
        auto nevras = libdnf::rpm::Nevra::parse("name.ar-ch", {libdnf::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL(0ul, nevras.size());
    }

    {
        auto nevras = libdnf::rpm::Nevra::parse("name.-arch", {libdnf::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL(0ul, nevras.size());
    }

    {
        auto nevras = libdnf::rpm::Nevra::parse("name.", {libdnf::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL(0ul, nevras.size());
    }

    {
        auto nevras = libdnf::rpm::Nevra::parse("four-of-fish-3.6.9.i686", {libdnf::rpm::Nevra::Form::NEV});
        CPPUNIT_ASSERT_EQUAL(1ul, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9.i686");
        CPPUNIT_ASSERT(nevra.get_release() == "");
        CPPUNIT_ASSERT(nevra.get_arch() == "");
    }

    // When parsing fails return false
    {
        auto nevras = libdnf::rpm::Nevra::parse("four-of-fish", {libdnf::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL(0ul, nevras.size());
    }

    // When parsing fails return false => '-' after '.'
    {
        auto nevras = libdnf::rpm::Nevra::parse("four-o.f-fish", {libdnf::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL(0ul, nevras.size());
    }

    // When parsing fails return false - not allowed characters '()'
    {
        CPPUNIT_ASSERT_THROW(
            libdnf::rpm::Nevra::parse("four-of(fish.i686)", {libdnf::rpm::Nevra::Form::NA}),
            libdnf::rpm::Nevra::IncorrectNevraString);
    }

    // Test parsing NEVRA with glob in epoch
    {
        auto nevras =
            libdnf::rpm::Nevra::parse("four-of-fish-[01]:3.6.9-11.fc100.x86_64", {libdnf::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL(1ul, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "[01]");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");
    }

    // Test parsing NEVRA with glob in epoch
    {
        auto nevras =
            libdnf::rpm::Nevra::parse("four-of-fish-?:3.6.9-11.fc100.x86_64", {libdnf::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL(1ul, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "?");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");
    }
}
