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


#include "test_repoquery.hpp"

#include "libdnf-cli/output/repoquery.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(RepoqueryTest);


void RepoqueryTest::setUp() {
    BaseTestCase::setUp();
    BaseTestCase::add_repo_repomd("repomd-repo1");
    pkgs = std::make_unique<libdnf::rpm::PackageQuery>(base);
}


void RepoqueryTest::test_format_set_with_simple_str() {
    FILE * stream = nullptr;
    char * buf = nullptr;
    size_t len = 0;
    stream = open_memstream(&buf, &len);

    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "test\n");
    CPPUNIT_ASSERT_EQUAL(fflush(stream), 0);

    CPPUNIT_ASSERT_EQUAL(std::string("test\n"), std::string(buf));

    free(buf);
}


void RepoqueryTest::test_format_set_with_tags() {
    FILE * stream = nullptr;
    char * buf = nullptr;
    size_t len = 0;

    // One tag
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%{name}\n");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(std::string("pkg\npkg-libs\nunresolvable\n"), std::string(buf));
    free(buf);

    // Duplicate tag
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%{name}-%{name}-%{name}\n");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(
        std::string("pkg-libs-pkg-libs-pkg-libs\npkg-pkg-pkg\nunresolvable-unresolvable-unresolvable\n"),
        std::string(buf));
    free(buf);

    // Different tags
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%{name}-%{arch}-%{installsize}\n");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(
        std::string("pkg-libs-x86_64-222\npkg-x86_64-222\nunresolvable-noarch-222\n"), std::string(buf));
    free(buf);
}


void RepoqueryTest::test_format_set_with_invalid_tags() {
    FILE * stream = nullptr;
    char * buf = nullptr;
    size_t len = 0;

    // Incomplete tag
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%{name\n");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(std::string("%{name\n"), std::string(buf));
    free(buf);

    // Not matching tag
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%{asd}\n");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(std::string("%{asd}\n"), std::string(buf));
    free(buf);

    // Incomplate tag with not matching tag and additional braces
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%%{ %% {{{%{asd}");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(std::string("%%{ %% {{{%{asd}"), std::string(buf));
    free(buf);

    // Incomplate tag with matching tag and additional braces
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%%{}{}{{%{name}}");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(std::string("%%{}{}{{pkg-libs}%%{}{}{{pkg}%%{}{}{{unresolvable}"), std::string(buf));
    free(buf);

    // fmt formatting strings are disabled
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%{name:^30}");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(std::string("%{name:^30}"), std::string(buf));
    free(buf);
}


void RepoqueryTest::test_format_set_with_tags_with_spacing() {
    FILE * stream = nullptr;
    char * buf = nullptr;
    size_t len = 0;

    // Tag with invalid format spec
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%aa20{name}%{evr}\n");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(std::string("%aa20{name}1.2-3\n%aa20{name}1:1.3-4\n%aa20{name}1:2-3\n"), std::string(buf));
    free(buf);

    // One tag with align spec
    stream = open_memstream(&buf, &len);
    libdnf::cli::output::print_pkg_set_with_format(stream, *pkgs, "%-20{name}%{evr}\n");
    CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
    CPPUNIT_ASSERT_EQUAL(
        std::string("pkg                 1.2-3\npkg-libs            1:1.3-4\nunresolvable        1:2-3\n"),
        std::string(buf));
    free(buf);
}
