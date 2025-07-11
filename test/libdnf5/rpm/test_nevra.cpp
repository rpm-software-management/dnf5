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

#include "test_nevra.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/rpm/nevra.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(NevraTest);


void NevraTest::setUp() {}


void NevraTest::tearDown() {}


void NevraTest::test_nevra() {
    {
        auto nevras =
            libdnf5::rpm::Nevra::parse("four-of-fish-8:3.6.9-11.fc100.x86_64", {libdnf5::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL((size_t)1, nevras.size());
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
            libdnf5::rpm::Nevra::parse("four-of-f:ish-3.6.9-11.fc100.x86_64", {libdnf5::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL((size_t)0, nevras.size());
    }

    // cannot parse due to ':' presence twice
    {
        CPPUNIT_ASSERT_THROW(
            libdnf5::rpm::Nevra::parse("four-of-fish-8:9:3.6.9-11.fc100.x86_64", {libdnf5::rpm::Nevra::Form::NEVRA}),
            libdnf5::rpm::NevraIncorrectInputError);
    }


    {
        auto nevras =
            libdnf5::rpm::Nevra::parse("four-of-fish-3.6.9-11.fc100.x86_64", {libdnf5::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL((size_t)1, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");

        // test that to_nevra_string() and to_full_nevra_string() template functions work without epoch
        CPPUNIT_ASSERT_EQUAL(std::string("four-of-fish-3.6.9-11.fc100.x86_64"), to_nevra_string(nevra));
        CPPUNIT_ASSERT_EQUAL(std::string("four-of-fish-0:3.6.9-11.fc100.x86_64"), to_full_nevra_string(nevra));
    }

    {
        auto nevras = libdnf5::rpm::Nevra::parse("four-of-fish-8:3.6.9", {libdnf5::rpm::Nevra::Form::NEV});
        CPPUNIT_ASSERT_EQUAL((size_t)1, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "8");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "");
        CPPUNIT_ASSERT(nevra.get_arch() == "");
    }

    {
        auto nevras = libdnf5::rpm::Nevra::parse("fish-8:3.6.9", {libdnf5::rpm::Nevra::Form::NEV});
        CPPUNIT_ASSERT_EQUAL((size_t)1, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT_EQUAL(std::string("fish"), nevra.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("8"), nevra.get_epoch());
        CPPUNIT_ASSERT_EQUAL(std::string("3.6.9"), nevra.get_version());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_release());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_arch());
    }

    {
        auto nevras = libdnf5::rpm::Nevra::parse("fish-3.6.9", {libdnf5::rpm::Nevra::Form::NEV});
        CPPUNIT_ASSERT_EQUAL((size_t)1, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT_EQUAL(std::string("fish"), nevra.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_epoch());
        CPPUNIT_ASSERT_EQUAL(std::string("3.6.9"), nevra.get_version());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_release());
        CPPUNIT_ASSERT_EQUAL(std::string(""), nevra.get_arch());
    }


    {
        auto nevras = libdnf5::rpm::Nevra::parse("four-of-fish-3.6.9.i686", {libdnf5::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL((size_t)1, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish-3.6.9");
        CPPUNIT_ASSERT(nevra.get_epoch() == "");
        CPPUNIT_ASSERT(nevra.get_version() == "");
        CPPUNIT_ASSERT(nevra.get_release() == "");
        CPPUNIT_ASSERT(nevra.get_arch() == "i686");
    }

    {
        auto nevras = libdnf5::rpm::Nevra::parse("name.ar-ch", {libdnf5::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL((size_t)0, nevras.size());
    }

    {
        auto nevras = libdnf5::rpm::Nevra::parse("name.-arch", {libdnf5::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL((size_t)0, nevras.size());
    }

    {
        auto nevras = libdnf5::rpm::Nevra::parse("name.", {libdnf5::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL((size_t)0, nevras.size());
    }

    {
        auto nevras = libdnf5::rpm::Nevra::parse("four-of-fish-3.6.9.i686", {libdnf5::rpm::Nevra::Form::NEV});
        CPPUNIT_ASSERT_EQUAL((size_t)1, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9.i686");
        CPPUNIT_ASSERT(nevra.get_release() == "");
        CPPUNIT_ASSERT(nevra.get_arch() == "");
    }

    // When parsing fails return false
    {
        auto nevras = libdnf5::rpm::Nevra::parse("four-of-fish", {libdnf5::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL((size_t)0, nevras.size());
    }

    // When parsing fails return false => '-' after '.'
    {
        auto nevras = libdnf5::rpm::Nevra::parse("four-o.f-fish", {libdnf5::rpm::Nevra::Form::NA});
        CPPUNIT_ASSERT_EQUAL((size_t)0, nevras.size());
    }

    // When parsing fails return false - not allowed characters '()'
    {
        CPPUNIT_ASSERT_THROW(
            libdnf5::rpm::Nevra::parse("four-of(fish.i686)", {libdnf5::rpm::Nevra::Form::NA}),
            libdnf5::rpm::NevraIncorrectInputError);
    }

    // Test parsing NEVRA with glob in epoch
    {
        auto nevras =
            libdnf5::rpm::Nevra::parse("four-of-fish-[01]:3.6.9-11.fc100.x86_64", {libdnf5::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL((size_t)1, nevras.size());
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
            libdnf5::rpm::Nevra::parse("four-of-fish-?:3.6.9-11.fc100.x86_64", {libdnf5::rpm::Nevra::Form::NEVRA});
        CPPUNIT_ASSERT_EQUAL((size_t)1, nevras.size());
        auto & nevra = *nevras.begin();
        CPPUNIT_ASSERT(nevra.get_name() == "four-of-fish");
        CPPUNIT_ASSERT(nevra.get_epoch() == "?");
        CPPUNIT_ASSERT(nevra.get_version() == "3.6.9");
        CPPUNIT_ASSERT(nevra.get_release() == "11.fc100");
        CPPUNIT_ASSERT(nevra.get_arch() == "x86_64");
    }
}


namespace {

class TestPackage : public libdnf5::rpm::Nevra {
public:
    explicit TestPackage(const char * nevra_str) {
        // use the static parse() method to parse `nevra_str` and then copy it to the current object
        auto nevra = libdnf5::rpm::Nevra::parse(nevra_str, {libdnf5::rpm::Nevra::Form::NEVRA}).at(0);
        libdnf5::rpm::copy_nevra_attributes(nevra, *this);
    }

    std::string get_evr() const {
        if (get_epoch().empty() || get_epoch() == "0") {
            return get_version() + "-" + get_release();
        }
        return get_epoch() + ":" + get_version() + "-" + get_release();
    }
};

}  // namespace


void NevraTest::test_evrcmp() {
    TestPackage foo_0_1_1_noarch("foo-1-1.noarch");
    TestPackage foo_1_1_1_noarch("foo-1:1-1.noarch");
    TestPackage foo_0_2_1_noarch("foo-2-1.noarch");
    TestPackage foo_0_1_2_noarch("foo-1-2.noarch");
    TestPackage foo_0_1_4_noarch("foo-1-4.noarch");
    TestPackage foo_0_1_1_1_noarch("foo-1.1-1.noarch");
    TestPackage bar__1_1_noarch("bar-1-1.noarch");
    TestPackage bar_0_1_1_noarch("bar-0:1-1.noarch");

    // compare empty epoch with zero epoch
    CPPUNIT_ASSERT_MESSAGE(
        "Empty and zero epoch are considered different.",
        libdnf5::rpm::cmp_nevra(bar__1_1_noarch, bar_0_1_1_noarch) == 0);

    // order by epoch
    std::vector<TestPackage> actual = {foo_1_1_1_noarch, foo_0_2_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_2_1_noarch, foo_1_1_1_noarch}), actual);

    // order by epoch - already ordered
    actual = {foo_0_2_1_noarch, foo_1_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_2_1_noarch, foo_1_1_1_noarch}), actual);

    // order by version
    actual = {foo_0_2_1_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_2_1_noarch}), actual);

    // order by version - already ordered
    actual = {foo_0_1_1_noarch, foo_0_2_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_2_1_noarch}), actual);

    // order by release
    actual = {foo_0_1_2_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_2_noarch}), actual);

    // order by release - already ordered
    actual = {foo_0_1_1_noarch, foo_0_1_2_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_2_noarch}), actual);

    // order by version (with minor version > release)
    actual = {foo_0_1_1_1_noarch, foo_0_1_4_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_4_noarch, foo_0_1_1_1_noarch}), actual);

    // order by version (with minor version > release) - already sorted
    actual = {foo_0_1_4_noarch, foo_0_1_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_4_noarch, foo_0_1_1_1_noarch}), actual);
}

void NevraTest::test_cmp_nevra() {
    TestPackage foo_0_1_1_noarch("foo-1-1.noarch");
    TestPackage foo_1_1_1_noarch("foo-1:1-1.noarch");
    TestPackage foo_0_2_1_noarch("foo-2-1.noarch");
    TestPackage foo_0_1_2_noarch("foo-1-2.noarch");
    TestPackage foo_0_1_1_x86_64("foo-1-1.x86_64");
    TestPackage foo_0_1_4_noarch("foo-1-4.noarch");
    TestPackage foo_0_1_1_1_noarch("foo-1.1-1.noarch");
    TestPackage bar_0_1_1_noarch("bar-1-1.noarch");

    // order by name
    std::vector<TestPackage> actual = {foo_0_1_1_noarch, bar_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({bar_0_1_1_noarch, foo_0_1_1_noarch}), actual);

    // order by name - already ordered
    actual = {bar_0_1_1_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({bar_0_1_1_noarch, foo_0_1_1_noarch}), actual);

    // order by epoch
    actual = {foo_1_1_1_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_1_1_1_noarch}), actual);

    // order by epoch - already ordered
    actual = {foo_0_1_1_noarch, foo_1_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_1_1_1_noarch}), actual);

    // order by version
    actual = {foo_0_2_1_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_2_1_noarch}), actual);

    // order by version - already ordered
    actual = {foo_0_1_1_noarch, foo_0_2_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_2_1_noarch}), actual);

    // order by release
    actual = {foo_0_1_2_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_2_noarch}), actual);

    // order by release - already ordered
    actual = {foo_0_1_1_noarch, foo_0_1_2_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_2_noarch}), actual);

    // order by arch
    actual = {foo_0_1_1_x86_64, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_1_x86_64}), actual);

    // order by arch - already ordered
    actual = {foo_0_1_1_noarch, foo_0_1_1_x86_64};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_1_x86_64}), actual);

    // order by version (with minor version > release)
    actual = {foo_0_1_1_1_noarch, foo_0_1_4_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_4_noarch, foo_0_1_1_1_noarch}), actual);

    // order by version (with minor version > release) - already sorted
    actual = {foo_0_1_4_noarch, foo_0_1_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_nevra<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_4_noarch, foo_0_1_1_1_noarch}), actual);
}


void NevraTest::test_cmp_naevr() {
    TestPackage foo_0_1_1_noarch("foo-1-1.noarch");
    TestPackage foo_1_1_1_noarch("foo-1:1-1.noarch");
    TestPackage foo_0_2_1_noarch("foo-2-1.noarch");
    TestPackage foo_0_1_2_noarch("foo-1-2.noarch");
    TestPackage foo_0_1_0_x86_64("foo-1-0.x86_64");
    TestPackage foo_0_1_4_noarch("foo-1-4.noarch");
    TestPackage foo_0_1_1_1_noarch("foo-1.1-1.noarch");
    TestPackage bar_0_1_1_noarch("bar-1-1.noarch");

    // order by name
    std::vector<TestPackage> actual = {foo_0_1_1_noarch, bar_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({bar_0_1_1_noarch, foo_0_1_1_noarch}), actual);

    // order by name - already ordered
    actual = {bar_0_1_1_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({bar_0_1_1_noarch, foo_0_1_1_noarch}), actual);

    // order by epoch
    actual = {foo_1_1_1_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_1_1_1_noarch}), actual);

    // order by epoch - already ordered
    actual = {foo_0_1_1_noarch, foo_1_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_1_1_1_noarch}), actual);

    // order by version
    actual = {foo_0_2_1_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_2_1_noarch}), actual);

    // order by version - already ordered
    actual = {foo_0_1_1_noarch, foo_0_2_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_2_1_noarch}), actual);

    // order by release
    actual = {foo_0_1_2_noarch, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_2_noarch}), actual);

    // order by release - already ordered
    actual = {foo_0_1_1_noarch, foo_0_1_2_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_2_noarch}), actual);

    // order by arch
    actual = {foo_0_1_0_x86_64, foo_0_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_0_x86_64}), actual);

    // order by arch - already ordered
    actual = {foo_0_1_1_noarch, foo_0_1_0_x86_64};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_1_noarch, foo_0_1_0_x86_64}), actual);

    // order by version (with minor version > release)
    actual = {foo_0_1_1_1_noarch, foo_0_1_4_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_4_noarch, foo_0_1_1_1_noarch}), actual);

    // order by version (with minor version > release) - already sorted
    actual = {foo_0_1_4_noarch, foo_0_1_1_1_noarch};
    std::sort(actual.begin(), actual.end(), libdnf5::rpm::cmp_naevr<TestPackage>);
    CPPUNIT_ASSERT_EQUAL(std::vector<TestPackage>({foo_0_1_4_noarch, foo_0_1_1_1_noarch}), actual);
}
