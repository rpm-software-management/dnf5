// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "test_repoquery.hpp"

#include <libdnf5-cli/output/repoquery.hpp>
#include <stdio.h>
#include <stdlib.h>

#include <string_view>
#include <system_error>

CPPUNIT_TEST_SUITE_REGISTRATION(RepoqueryTest);


void RepoqueryTest::setUp() {
    BaseTestCase::setUp();
    BaseTestCase::add_repo_repomd("repomd-repo1");
    pkgs = std::make_unique<libdnf5::rpm::PackageQuery>(base);
}


namespace {

class MemStream {
public:
    MemStream() {
        stream = open_memstream(&buf, &len);
        if (!stream) {
            throw std::system_error(errno, std::system_category(), "open_memstream");
        }
    }

    ~MemStream() {
        CPPUNIT_ASSERT_EQUAL(fclose(stream), 0);
        free(buf);
    }

    FILE * get_file() noexcept { return stream; }

    std::string_view get_string_view() noexcept {
        CPPUNIT_ASSERT_EQUAL(fflush(stream), 0);
        return std::string_view(buf, len);
    }

private:
    FILE * stream;
    char * buf;
    size_t len;
};

}  // namespace


void RepoqueryTest::test_format_set_with_simple_str() {
    MemStream stream;
    libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "test\n");
    CPPUNIT_ASSERT_EQUAL(std::string_view("test\n"), stream.get_string_view());
}


void RepoqueryTest::test_format_set_with_tags() {
    // One tag
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%{name}\n");
        CPPUNIT_ASSERT_EQUAL(std::string_view("pkg\npkg-libs\nunresolvable\n"), stream.get_string_view());
    }

    // Duplicate tag
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%{name}-%{name}-%{name}\n");
        CPPUNIT_ASSERT_EQUAL(
            std::string_view("pkg-libs-pkg-libs-pkg-libs\npkg-pkg-pkg\nunresolvable-unresolvable-unresolvable\n"),
            stream.get_string_view());
    }

    // Different tags
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%{name}-%{arch}-%{installsize}\n");
        CPPUNIT_ASSERT_EQUAL(
            std::string_view("pkg-libs-x86_64-222\npkg-x86_64-222\nunresolvable-noarch-222\n"),
            stream.get_string_view());
    }
}


void RepoqueryTest::test_format_set_with_invalid_tags() {
    // Incomplete tag
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%{name\n");
        CPPUNIT_ASSERT_EQUAL(std::string_view("%{name\n"), stream.get_string_view());
    }

    // Not matching tag
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%{asd}\n");
        CPPUNIT_ASSERT_EQUAL(std::string_view("%{asd}\n"), stream.get_string_view());
    }

    // Incomplate tag with not matching tag and additional braces
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%%{ %% {{{%{asd}");
        CPPUNIT_ASSERT_EQUAL(std::string_view("%%{ %% {{{%{asd}"), stream.get_string_view());
    }

    // Incomplate tag with matching tag and additional braces
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%%{}{}{{%{name}}");
        CPPUNIT_ASSERT_EQUAL(
            std::string_view("%%{}{}{{pkg-libs}%%{}{}{{pkg}%%{}{}{{unresolvable}"), stream.get_string_view());
    }

    // fmt formatting strings are disabled
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%{name:^30}");
        CPPUNIT_ASSERT_EQUAL(std::string_view("%{name:^30}"), stream.get_string_view());
    }
}


void RepoqueryTest::test_format_set_with_tags_with_spacing() {
    // Tag with invalid format spec
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%aa20{name}%{evr}\n");
        CPPUNIT_ASSERT_EQUAL(
            std::string_view("%aa20{name}1.2-3\n%aa20{name}1:1.3-4\n%aa20{name}1:2-3\n"), stream.get_string_view());
    }

    // One tag with align spec
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_set_with_format(stream.get_file(), *pkgs, "%-20{name}%{evr}\n");
        CPPUNIT_ASSERT_EQUAL(
            std::string_view("pkg                 1.2-3\npkg-libs            1:1.3-4\nunresolvable        1:2-3\n"),
            stream.get_string_view());
    }
}


void RepoqueryTest::test_pkg_attr_uniq_sorted() {
    // requires
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_attr_uniq_sorted(stream.get_file(), *pkgs, "requires");
        CPPUNIT_ASSERT_EQUAL(std::string_view("prereq\nreq = 1:2-3\n"), stream.get_string_view());
    }

    // provides
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_attr_uniq_sorted(stream.get_file(), *pkgs, "provides");
        CPPUNIT_ASSERT_EQUAL(
            std::string_view("pkg = 1.2-3\npkg-libs = 1:1.3-4\nprv = 1:2-3\nunresolvable = 1:2-3\n"),
            stream.get_string_view());
    }

    // name
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_attr_uniq_sorted(stream.get_file(), *pkgs, "name");
        CPPUNIT_ASSERT_EQUAL(std::string_view("pkg\npkg-libs\nunresolvable\n"), stream.get_string_view());
    }

    // deduplicated buildtimes
    {
        MemStream stream;
        libdnf5::cli::output::print_pkg_attr_uniq_sorted(stream.get_file(), *pkgs, "buildtime");
        CPPUNIT_ASSERT_EQUAL(std::string_view("456\n"), stream.get_string_view());
    }
}


void RepoqueryTest::test_requires_filelists() {
    CPPUNIT_ASSERT_EQUAL(libdnf5::cli::output::requires_filelists("asd"), false);
    CPPUNIT_ASSERT_EQUAL(libdnf5::cli::output::requires_filelists("file"), false);
    CPPUNIT_ASSERT_EQUAL(libdnf5::cli::output::requires_filelists("%{file}"), false);
    CPPUNIT_ASSERT_EQUAL(libdnf5::cli::output::requires_filelists("%{name}"), false);
    CPPUNIT_ASSERT_EQUAL(libdnf5::cli::output::requires_filelists("%{files}"), true);
}
