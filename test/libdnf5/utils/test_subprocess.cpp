// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "test_subprocess.hpp"

#include "utils/subprocess.hpp"

#include <cstring>


CPPUNIT_TEST_SUITE_REGISTRATION(UtilsSubprocessTest);

// These tests call only /usr/bin/env sh and use only its builtins, so there shouldn't be any dependency/portability problems.

void UtilsSubprocessTest::setUp() {}


void UtilsSubprocessTest::tearDown() {}

// Helper function to convert std::vector<std::byte> to std::string
static std::string bytes_to_string(const std::vector<std::byte> & data) {
    std::string result;
    result.reserve(data.size());
    for (const auto byte : data) {
        result.push_back(static_cast<char>(byte));
    }
    return result;
}


void UtilsSubprocessTest::test_stdout_capture() {
    // Test that stdout is captured correctly
    const auto result =
        libdnf5::utils::subprocess::run("/usr/bin/env", {"/usr/bin/env", "sh", "-c", "echo 'Hello, World!'"});

    CPPUNIT_ASSERT_EQUAL(0, result.returncode);
    CPPUNIT_ASSERT_EQUAL(std::string("Hello, World!\n"), bytes_to_string(result.stdout));
    CPPUNIT_ASSERT_EQUAL(std::string(""), bytes_to_string(result.stderr));
}


void UtilsSubprocessTest::test_stderr_capture() {
    // Test that stderr is captured correctly
    const auto result =
        libdnf5::utils::subprocess::run("/usr/bin/env", {"/usr/bin/env", "sh", "-c", "echo 'Error message' >&2"});

    CPPUNIT_ASSERT_EQUAL(0, result.returncode);
    CPPUNIT_ASSERT_EQUAL(std::string(""), bytes_to_string(result.stdout));
    CPPUNIT_ASSERT_EQUAL(std::string("Error message\n"), bytes_to_string(result.stderr));
}


void UtilsSubprocessTest::test_stdout_and_stderr_capture() {
    // Test that both stdout and stderr are captured correctly
    const auto result = libdnf5::utils::subprocess::run(
        "/usr/bin/env", {"/usr/bin/env", "sh", "-c", "echo 'stdout line'; echo 'stderr line' >&2"});

    CPPUNIT_ASSERT_EQUAL(0, result.returncode);
    CPPUNIT_ASSERT_EQUAL(std::string("stdout line\n"), bytes_to_string(result.stdout));
    CPPUNIT_ASSERT_EQUAL(std::string("stderr line\n"), bytes_to_string(result.stderr));
}


void UtilsSubprocessTest::test_exit_code_success() {
    // Test that exit code 0 is captured correctly, empty stdout, empty stderr
    const auto result = libdnf5::utils::subprocess::run("/usr/bin/env", {"/usr/bin/env", "sh", "-c", "exit 0"});

    CPPUNIT_ASSERT_EQUAL(0, result.returncode);
    CPPUNIT_ASSERT_EQUAL(std::string(""), bytes_to_string(result.stdout));
    CPPUNIT_ASSERT_EQUAL(std::string(""), bytes_to_string(result.stderr));
}


void UtilsSubprocessTest::test_exit_code_failure() {
    // Test that non-zero exit codes are captured correctly
    const auto result = libdnf5::utils::subprocess::run("/usr/bin/env", {"/usr/bin/env", "sh", "-c", "exit 42"});

    CPPUNIT_ASSERT_EQUAL(42, result.returncode);
}


void UtilsSubprocessTest::test_signal_termination() {
    // Test that signal termination is captured as negative return code
    // SIGTERM is signal 15, so we expect returncode to be -15
    const auto result = libdnf5::utils::subprocess::run("/usr/bin/env", {"/usr/bin/env", "sh", "-c", "kill -TERM $$"});

    CPPUNIT_ASSERT_EQUAL(-15, result.returncode);
}


void UtilsSubprocessTest::test_binary_data() {
    // Test that binary data (non-text) is captured correctly
    // Use printf to output bytes including null bytes and non-printable characters
    const auto result = libdnf5::utils::subprocess::run(
        "/usr/bin/env", {"/usr/bin/env", "sh", "-c", "printf '\\x00\\x01\\x02\\xff\\xfe'; printf '\\x03\\x04' >&2"});

    CPPUNIT_ASSERT_EQUAL(0, result.returncode);

    // Verify stdout contains the expected binary data
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), result.stdout.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>(0x00), result.stdout[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>(0x01), result.stdout[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>(0x02), result.stdout[2]);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>(0xff), result.stdout[3]);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>(0xfe), result.stdout[4]);

    // Verify stderr contains the expected binary data
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.stderr.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>(0x03), result.stderr[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>(0x04), result.stderr[1]);
}


void UtilsSubprocessTest::test_interleaved_io_no_deadlock() {
    // Test that large writes to both stdout and stderr don't cause deadlock
    // The typical pipe buffer on Linux is 64KB. We write significantly more than that
    // to both stdout and stderr to ensure the pipes fill up.

    // Generate 100KB on stdout (1000 × 100 bytes)
    // Generate 100KB on stderr (1000 × 100 bytes)
    // Both printfs are in the same loop to ensure IO is definitely interleaved
    const std::string script = R"(
i=0
while [ $i -lt 1000 ]; do
    printf 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'
    printf 'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb' >&2
    i=$((i + 1))
done
)";

    const auto result = libdnf5::utils::subprocess::run("/usr/bin/env", {"/usr/bin/env", "sh", "-c", script});

    // The expected size is: 1000 × 100 bytes = 100,000 bytes per stream
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(100000), result.stdout.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(100000), result.stderr.size());

    // Verify the command succeeded (didn't deadlock or error)
    CPPUNIT_ASSERT_EQUAL(0, result.returncode);

    // Verify the data is correct (should be all a's on stdout, b's on stderr)
    // Check first few bytes (only if we have data)
    if (result.stdout.size() > 0) {
        CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>('a'), result.stdout[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>('a'), result.stdout[100]);
    }
    if (result.stderr.size() > 0) {
        CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>('b'), result.stderr[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::byte>('b'), result.stderr[100]);
    }
}
