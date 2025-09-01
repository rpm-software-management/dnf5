// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of DNF5: https://github.com/rpm-software-management/dnf5/
//
// DNF5 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// DNF5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DNF5.  If not, see <https://www.gnu.org/licenses/>.


#include "test_url.hpp"

#include "utils/url.hpp"


using namespace libdnf5::utils::url;


CPPUNIT_TEST_SUITE_REGISTRATION(UtilsUrlTest);


void UtilsUrlTest::setUp() {}


void UtilsUrlTest::tearDown() {}


void UtilsUrlTest::test_url_encode() {
    // Test basic alphanumeric characters (should not be encoded)
    CPPUNIT_ASSERT_EQUAL(std::string("abc123ABC"), url_encode("abc123ABC"));

    // Test unreserved characters (should not be encoded)
    CPPUNIT_ASSERT_EQUAL(std::string("abc-._~"), url_encode("abc-._~"));

    // Test space encoding
    CPPUNIT_ASSERT_EQUAL(std::string("hello%20world"), url_encode("hello world"));

    // Test common special characters
    CPPUNIT_ASSERT_EQUAL(std::string("%2b"), url_encode("+"));  // plus
    CPPUNIT_ASSERT_EQUAL(std::string("%26"), url_encode("&"));  // ampersand
    CPPUNIT_ASSERT_EQUAL(std::string("%3d"), url_encode("="));  // equals
    CPPUNIT_ASSERT_EQUAL(std::string("%3f"), url_encode("?"));  // question mark
    CPPUNIT_ASSERT_EQUAL(std::string("%23"), url_encode("#"));  // hash
    CPPUNIT_ASSERT_EQUAL(std::string("%2f"), url_encode("/"));  // slash
    CPPUNIT_ASSERT_EQUAL(std::string("%3a"), url_encode(":"));  // colon

    // Test mixed content
    CPPUNIT_ASSERT_EQUAL(std::string("user%2bname%40example.com"), url_encode("user+name@example.com"));

    // Test empty string
    CPPUNIT_ASSERT_EQUAL(std::string(""), url_encode(""));

    // Test string with only special characters
    CPPUNIT_ASSERT_EQUAL(std::string("%20%2b%26"), url_encode(" +&"));
}


void UtilsUrlTest::test_url_decode() {
    // Test basic decoding
    CPPUNIT_ASSERT_EQUAL(std::string("hello world"), url_decode("hello%20world"));
    CPPUNIT_ASSERT_EQUAL(std::string("abc123ABC"), url_decode("abc123ABC"));

    // Test unreserved characters (should pass through)
    CPPUNIT_ASSERT_EQUAL(std::string("abc-._~"), url_decode("abc-._~"));

    // Test common special characters
    CPPUNIT_ASSERT_EQUAL(std::string("+"), url_decode("%2b"));  // plus
    CPPUNIT_ASSERT_EQUAL(std::string("+"), url_decode("%2B"));  // plus (uppercase)
    CPPUNIT_ASSERT_EQUAL(std::string("&"), url_decode("%26"));  // ampersand
    CPPUNIT_ASSERT_EQUAL(std::string("="), url_decode("%3d"));  // equals
    CPPUNIT_ASSERT_EQUAL(std::string("="), url_decode("%3D"));  // equals (uppercase)
    CPPUNIT_ASSERT_EQUAL(std::string("?"), url_decode("%3f"));  // question mark
    CPPUNIT_ASSERT_EQUAL(std::string("#"), url_decode("%23"));  // hash
    CPPUNIT_ASSERT_EQUAL(std::string("/"), url_decode("%2f"));  // slash
    CPPUNIT_ASSERT_EQUAL(std::string(":"), url_decode("%3a"));  // colon

    // Test mixed content
    CPPUNIT_ASSERT_EQUAL(std::string("user+name@example.com"), url_decode("user%2bname%40example.com"));

    // Test empty string
    CPPUNIT_ASSERT_EQUAL(std::string(""), url_decode(""));

    // Test malformed encoding
    CPPUNIT_ASSERT_EQUAL(std::string("%"), url_decode("%"));
    CPPUNIT_ASSERT_EQUAL(std::string("%1"), url_decode("%1"));
    CPPUNIT_ASSERT_EQUAL(std::string("%1g"), url_decode("%1g"));
}


void UtilsUrlTest::test_encode_decode_roundtrip() {
    // Test roundtrip encoding/decoding
    std::vector<std::string> test_cases = {
        "",
        "hello world",
        "user+name@example.com",
        "path/with spaces/and+symbols",
        "query=value&other=data",
        "special chars: !@#$%^&*()",
        "unicode test: café",
    };

    for (const auto & original : test_cases) {
        std::string encoded = url_encode(original);
        std::string decoded = url_decode(encoded);
        CPPUNIT_ASSERT_EQUAL(original, decoded);
    }
}


void UtilsUrlTest::test_url_path_encode() {
    // Test empty string
    CPPUNIT_ASSERT_EQUAL(std::string(""), url_path_encode("", false));
    CPPUNIT_ASSERT_EQUAL(std::string(""), url_path_encode("", true));

    // Test single separator
    CPPUNIT_ASSERT_EQUAL(std::string("/"), url_path_encode("/", false));
    CPPUNIT_ASSERT_EQUAL(std::string("/"), url_path_encode("/", true));

    // Test only separators
    CPPUNIT_ASSERT_EQUAL(std::string("///"), url_path_encode("///", false));

    // Test path with only spaces
    CPPUNIT_ASSERT_EQUAL(std::string("%20/%20/%20"), url_path_encode(" / / ", false));

    // Test basic path encoding - preserves "/" separators
    CPPUNIT_ASSERT_EQUAL(
        std::string("path/with%20spaces/file%20name.txt"), url_path_encode("path/with spaces/file name.txt", false));

    // Test path with special characters
    CPPUNIT_ASSERT_EQUAL(
        std::string("path/with%2bsymbols%26chars/file"), url_path_encode("path/with+symbols&chars/file", false));

    // Test unreserved characters (should not be encoded)
    CPPUNIT_ASSERT_EQUAL(std::string("abc123-._~/path"), url_path_encode("abc123-._~/path", false));

    CPPUNIT_ASSERT_EQUAL(std::string("/start/path/end/"), url_path_encode("/start/path/end/", false));

    // Test multiple consecutive separators
    CPPUNIT_ASSERT_EQUAL(std::string("path//double//separators"), url_path_encode("path//double//separators", false));

    // Test preserve_already_encoded = false (may double-encode)
    CPPUNIT_ASSERT_EQUAL(
        std::string("path/with%2520spaces/file%2520name.txt"),
        url_path_encode("path/with%20spaces/file%20name.txt", false));

    // Test preserve_already_encoded = true (normalize encoding)
    CPPUNIT_ASSERT_EQUAL(
        std::string("path/with%20spaces/file%20name.txt"), url_path_encode("path/with%20spaces/file%20name.txt", true));

    // Test mixed encoded and unencoded with preserve_already_encoded = true
    CPPUNIT_ASSERT_EQUAL(
        std::string("path/encoded%20part/new%20part"), url_path_encode("path/encoded%20part/new part", true));

    // Test mixed encoded and unencoded with preserve_already_encoded = false
    CPPUNIT_ASSERT_EQUAL(
        std::string("path/encoded%2520part/new%20part"), url_path_encode("path/encoded%20part/new part", false));

    // Test fully unencoded path with preserve flag (should behave the same)
    CPPUNIT_ASSERT_EQUAL(std::string("path/with%20spaces"), url_path_encode("path/with spaces", true));
    CPPUNIT_ASSERT_EQUAL(std::string("path/with%20spaces"), url_path_encode("path/with spaces", false));

    // Test malformed encoding with preserve flag
    CPPUNIT_ASSERT_EQUAL(
        std::string("path/with%25incomplete/encoding"), url_path_encode("path/with%incomplete/encoding", true));

    // Test repository-style paths (common use case)
    CPPUNIT_ASSERT_EQUAL(
        std::string("repo/path/package%2bname-1.0.rpm"), url_path_encode("repo/path/package+name-1.0.rpm", false));
}

void UtilsUrlTest::test_is_url() {
    // Test valid URLs
    CPPUNIT_ASSERT_EQUAL(true, is_url("http://example.com"));
    CPPUNIT_ASSERT_EQUAL(true, is_url("https://example.com"));
    CPPUNIT_ASSERT_EQUAL(true, is_url("ftp://ftp.example.com"));
    CPPUNIT_ASSERT_EQUAL(true, is_url("file:///path/to/file"));

    // Test case insensitive
    CPPUNIT_ASSERT_EQUAL(true, is_url("HTTP://EXAMPLE.COM"));
    CPPUNIT_ASSERT_EQUAL(true, is_url("FtP://example.com"));

    // Test invalid URLs
    CPPUNIT_ASSERT_EQUAL(false, is_url(""));
    CPPUNIT_ASSERT_EQUAL(false, is_url("example.com"));
    CPPUNIT_ASSERT_EQUAL(false, is_url("/path/to/file"));
    CPPUNIT_ASSERT_EQUAL(false, is_url("relative/path"));
    CPPUNIT_ASSERT_EQUAL(false, is_url("not-a-url"));
}
