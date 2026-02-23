// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

// This test verifies that tutorial plugin templates compile and load correctly.
// It runs the compiled dnf5 binary with --version flag and verifies that:
//   1. Both tutorial plugins are successfully loaded (Plugin names appear in the version output):
//      - libdnf5-plugin template (libdnf5_template_plugin)
//      - dnf5-plugin template (dnf5_template_plugin)
//   2. The dnf5 binary exits with status code 0

#include "test_tutorial_templates.hpp"

#include <fcntl.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <string>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION(TutorialTemplatesTest);


void TutorialTemplatesTest::setUp() {
    temp = std::make_unique<libdnf5::utils::fs::TempDir>("libdnf_unittest");
    installroot = temp->get_path() / "installroot";
    cachedir = installroot / "var/cache/dnf/";
}


void TutorialTemplatesTest::tearDown() {}


namespace {
constexpr int PIPE_READ = 0;
constexpr int PIPE_WRITE = 1;
}  // namespace


void TutorialTemplatesTest::dnf5() {
    int pipe_to_dnf[2];
    int pipe_from_dnf[2];
    int pipe_err_from_dnf[2];
    int ret = pipe2(pipe_to_dnf, O_CLOEXEC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("pipe creation failed", 0, ret);
    ret = pipe2(pipe_from_dnf, O_CLOEXEC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("pipe creation failed", 0, ret);
    ret = pipe2(pipe_err_from_dnf, O_CLOEXEC);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("pipe creation failed", 0, ret);

    const auto dnf_pid = fork();
    if (dnf_pid == 0) {
        close(pipe_to_dnf[PIPE_WRITE]);
        close(pipe_from_dnf[PIPE_READ]);
        close(pipe_err_from_dnf[PIPE_READ]);

        // bind stdin of the child process to the reading end of the pipe
        if (dup2(pipe_to_dnf[PIPE_READ], fileno(stdin)) == -1) {
            _exit(255);
        }
        close(pipe_to_dnf[PIPE_READ]);

        // bind stdout of the child process to the writing end of the pipe
        if (dup2(pipe_from_dnf[PIPE_WRITE], fileno(stdout)) == -1) {
            _exit(255);
        }
        close(pipe_from_dnf[PIPE_WRITE]);

        // bind stderr of the child process to the writing end of the pipe
        if (dup2(pipe_err_from_dnf[PIPE_WRITE], fileno(stderr)) == -1) {
            _exit(255);
        }
        close(pipe_err_from_dnf[PIPE_WRITE]);

        // Prepare environment with DNF5_PLUGIN_DIR
        std::vector<std::string> env_storage;
        env_storage.emplace_back(std::string("DNF5_PLUGINS_DIR=") + DNF5_PLUGIN_DIR);
        // Build pointer array
        std::vector<char *> env_ptrs;
        for (auto & env_str : env_storage) {
            env_ptrs.push_back(env_str.data());
        }
        env_ptrs.push_back(nullptr);

        // Build plugin arguments
        std::string pluginconfpath_arg = std::string("--setopt=plugin_conf_dir=") + LIBDNF5_PLUGIN_DIR;
        std::string pluginpath_arg = std::string("--setopt=pluginpath=") + LIBDNF5_PLUGIN_DIR;
        std::string installroot_arg = std::string("--installroot=") + installroot.string();

        // Execute dnf5
        execle(
            DNF5_BINARY_PATH,
            "dnf5",
            installroot_arg.c_str(),
            pluginconfpath_arg.c_str(),
            pluginpath_arg.c_str(),
            "--version",
            nullptr,
            env_ptrs.data());
    } else {
        close(pipe_to_dnf[PIPE_READ]);
        close(pipe_from_dnf[PIPE_WRITE]);
        close(pipe_err_from_dnf[PIPE_WRITE]);

        // Read from both stdout and stderr using poll() to avoid deadlock
        char read_buf[256];
        std::string dnf_stdout_content;
        int dnf_out_fd = pipe_from_dnf[PIPE_READ];
        int dnf_err_fd = pipe_err_from_dnf[PIPE_READ];

        struct pollfd fds[2];
        fds[0].fd = dnf_out_fd;
        fds[0].events = POLLIN;
        fds[1].fd = dnf_err_fd;
        fds[1].events = POLLIN;

        while (fds[0].fd != -1 || fds[1].fd != -1) {
            int ret = poll(fds, 2, -1);
            if (ret <= 0) {
                break;
            }

            // Check stdout
            if (fds[0].fd != -1 && (fds[0].revents & POLLIN)) {
                auto len = read(dnf_out_fd, read_buf, sizeof(read_buf));
                if (len > 0) {
                    dnf_stdout_content.append(read_buf, static_cast<std::size_t>(len));
                } else {
                    fds[0].fd = -1;  // Mark as closed, poll() will ignore it
                }
            }
            if (fds[0].fd != -1 && (fds[0].revents & (POLLHUP | POLLERR))) {
                fds[0].fd = -1;  // Mark as closed
            }

            // Check stderr (discard content)
            if (fds[1].fd != -1 && (fds[1].revents & POLLIN)) {
                auto len = read(dnf_err_fd, read_buf, sizeof(read_buf));
                if (len <= 0) {
                    fds[1].fd = -1;  // Mark as closed
                }
                // stderr content is discarded
            }
            if (fds[1].fd != -1 && (fds[1].revents & (POLLHUP | POLLERR))) {
                fds[1].fd = -1;  // Mark as closed
            }
        }

        // Verify that both plugins are present in the output
        CPPUNIT_ASSERT_MESSAGE(
            "dnf5_template_plugin not found in output",
            dnf_stdout_content.find("name: dnf5_template_plugin") != std::string::npos);
        CPPUNIT_ASSERT_MESSAGE(
            "libdnf5_template_plugin not found in output",
            dnf_stdout_content.find("name: libdnf5_template_plugin") != std::string::npos);

        close(pipe_to_dnf[PIPE_WRITE]);
        close(pipe_from_dnf[PIPE_READ]);
        close(pipe_err_from_dnf[PIPE_READ]);

        // Wait for dnf5 process to finish and check exit status
        int status;
        waitpid(dnf_pid, &status, 0);
        CPPUNIT_ASSERT_MESSAGE("dnf5 process did not exit normally", WIFEXITED(status));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("dnf5 exit code was not 0", 0, WEXITSTATUS(status));
    }
}
