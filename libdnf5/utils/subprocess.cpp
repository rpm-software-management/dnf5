// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "utils/subprocess.hpp"

#include "utils/string.hpp"

#include <fcntl.h>
#include <libdnf5/common/exception.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <string>
#include <utility>
#include <vector>

namespace {

class Pipe {
public:
    Pipe() {
        if (pipe2(fds, O_CLOEXEC) == -1) {
            throw libdnf5::SystemError(errno, M_("cannot create pipe"));
        }
    }

    Pipe(const Pipe &) = delete;
    Pipe & operator=(const Pipe &) = delete;

    Pipe(Pipe && other) noexcept { *this = std::move(other); }

    Pipe & operator=(Pipe && pipe) noexcept {
        if (this != &pipe) {
            fds[PipeEnd::READ] = pipe.fds[PipeEnd::READ];
            fds[PipeEnd::WRITE] = pipe.fds[PipeEnd::WRITE];
            pipe.fds[PipeEnd::READ] = -1;
            pipe.fds[PipeEnd::WRITE] = -1;
        }
        return *this;
    }

    int get_in() const noexcept { return fds[PipeEnd::READ]; }
    int get_out() const noexcept { return fds[PipeEnd::WRITE]; }

    void close_in() noexcept { close(PipeEnd::READ); }
    void close_out() noexcept { close(PipeEnd::WRITE); }

    ~Pipe() {
        close_in();
        close_out();
    }

private:
    enum PipeEnd { READ = 0, WRITE = 1 };

    void close(int fd_idx) noexcept {
        if (fds[fd_idx] != -1) {
            ::close(fds[fd_idx]);
            fds[fd_idx] = -1;
        }
    }

    int fds[2];
};

/// The class template OnScopeExit is a general-purpose scope guard
/// intended to call its exit function when a scope is exited.
template <typename TExitFunction>
    requires requires(TExitFunction f) {
        { f() } noexcept;
    }
class OnScopeExit {
public:
    OnScopeExit(TExitFunction && function) noexcept : exit_function{std::move(function)} {}

    ~OnScopeExit() noexcept { exit_function(); }

    OnScopeExit(const OnScopeExit &) = delete;
    OnScopeExit(OnScopeExit &&) = delete;
    OnScopeExit & operator=(const OnScopeExit &) = delete;
    OnScopeExit & operator=(OnScopeExit &&) = delete;

private:
    TExitFunction exit_function;
};

}  // namespace

namespace libdnf5::utils {

CompletedProcess run(std::string command, std::vector<std::string> args) {
    // Struct is used to pass a possible error from a child process before starting a new program.
    struct ErrorMessage {
        enum { BIND_STDOUT, BIND_STDERR, EXEC } error;  // what failed
        int err_code;                                   // errno
    };

    Pipe pipe_error_msg_from_child;
    Pipe pipe_out_from_child;
    Pipe pipe_err_from_child;

    // Prepare a null-terminated array of arguments for the exec procedure.
    // We don't want to risk throwing an exception in the child process, so we prepare it here.
    std::vector<char *> c_args;
    c_args.reserve(args.size() + 1);
    for (auto & arg : args) {
        c_args.push_back(arg.data());
    }
    c_args.push_back(nullptr);

    int child_exit_status;

    const auto child_pid = fork();
    if (child_pid == -1) {
        throw SystemError(errno, M_("cannot fork"));
    }

    if (child_pid == 0) {
        pipe_error_msg_from_child.close_in();
        pipe_out_from_child.close_in();  // close reading end of the pipe on the child side
        pipe_err_from_child.close_in();  // close reading end of the pipe on the child side

        // bind stdout of the child process to the writing end of the pipe
        if (dup2(pipe_out_from_child.get_out(), fileno(stdout)) == -1) {
            ErrorMessage msg{ErrorMessage::BIND_STDOUT, errno};
            if (write(pipe_error_msg_from_child.get_out(), &msg, sizeof(msg)) != sizeof(msg)) {
                // Ignore errors generated when sending an error.
                // The process terminates which is detected as an error in the parent process anyway.
            }
            _exit(255);
        }
        pipe_out_from_child.close_out();

        // bind stderr of the child process to the writing end of the pipe
        if (dup2(pipe_err_from_child.get_out(), fileno(stderr)) == -1) {
            ErrorMessage msg{ErrorMessage::BIND_STDERR, errno};
            if (write(pipe_error_msg_from_child.get_out(), &msg, sizeof(msg)) != sizeof(msg)) {
            }
            _exit(255);
        }
        pipe_err_from_child.close_out();

        execvp(command.c_str(), c_args.data());  // replace the child process with the command
        ErrorMessage msg{ErrorMessage::EXEC, errno};
        if (write(pipe_error_msg_from_child.get_out(), &msg, sizeof(msg)) != sizeof(msg)) {
        }
        _exit(255);
    }

    OnScopeExit finish([&pipe_error_msg_from_child, &pipe_out_from_child, &pipe_err_from_child]() noexcept {
        pipe_error_msg_from_child.close_in();
        pipe_out_from_child.close_in();
        pipe_err_from_child.close_in();
    });

    pipe_error_msg_from_child.close_out();
    pipe_out_from_child.close_out();
    pipe_err_from_child.close_out();

    // Check the pipe for errors. The child process will close it empty or write an error.
    ErrorMessage err_msg;
    auto ret = read(pipe_error_msg_from_child.get_in(), &err_msg, sizeof(err_msg));
    if (ret == sizeof(err_msg)) {
        switch (err_msg.error) {
            case ErrorMessage::BIND_STDOUT:
                throw SystemError(err_msg.err_code, M_("cannot bind child stdout"));
            case ErrorMessage::BIND_STDERR:
                throw SystemError(err_msg.err_code, M_("cannot bind child stderr"));
            case ErrorMessage::EXEC:
                throw SystemError(
                    err_msg.err_code, M_("failed to execute command: {}"), libdnf5::utils::string::join(args, " "));
        }
    } else if (ret != 0) {
        throw SystemError(
            err_msg.err_code,
            M_("error during preparation of child process: {}"),
            libdnf5::utils::string::join(args, " "));
    }
    pipe_error_msg_from_child.close_in();

    // Use poll() to multiplex I/O operations: read from stdout and stderr concurrently
    // This prevents issues when the child blocks on writing to one stream while we're reading the other
    std::vector<std::byte> stdout_bytes;
    std::vector<std::byte> stderr_bytes;

    // Set up poll file descriptors
    constexpr int STDOUT_FD = 0;
    constexpr int STDERR_FD = 1;

    std::array<pollfd, 2> fds{};
    fds[STDOUT_FD] = {.fd = pipe_out_from_child.get_in(), .events = POLLIN, .revents = 0};
    fds[STDERR_FD] = {.fd = pipe_err_from_child.get_in(), .events = POLLIN, .revents = 0};

    char buffer[4096];

    // Continue polling while we have active file descriptors
    while (fds[STDOUT_FD].fd != -1 || fds[STDERR_FD].fd != -1) {
        const int poll_result = poll(fds.data(), fds.size(), -1);  // -1 = wait indefinitely

        if (poll_result == -1) {
            if (errno == EINTR) {
                continue;  // Interrupted by signal, retry
            }
            throw SystemError(errno, M_("poll failed during subprocess I/O"));
        }

        if (fds[STDOUT_FD].revents & POLLIN) {
            const ssize_t bytes_read = read(fds[STDOUT_FD].fd, buffer, sizeof(buffer));
            if (bytes_read > 0) {
                stdout_bytes.insert(
                    stdout_bytes.end(),
                    reinterpret_cast<std::byte *>(buffer),
                    reinterpret_cast<std::byte *>(buffer + bytes_read));
            } else if (bytes_read == 0) {
                fds[STDOUT_FD].fd = -1;
            } else if (errno != EINTR) {
                fds[STDOUT_FD].fd = -1;
            }
        }

        if (fds[STDERR_FD].revents & POLLIN) {
            const ssize_t bytes_read = read(fds[STDERR_FD].fd, buffer, sizeof(buffer));
            if (bytes_read > 0) {
                stderr_bytes.insert(
                    stderr_bytes.end(),
                    reinterpret_cast<std::byte *>(buffer),
                    reinterpret_cast<std::byte *>(buffer + bytes_read));
            } else if (bytes_read == 0) {
                fds[STDERR_FD].fd = -1;
            } else if (errno != EINTR) {
                fds[STDERR_FD].fd = -1;
            }
        }

        // Check for errors or hangup on any descriptor
        if (fds[STDOUT_FD].revents & (POLLERR | POLLHUP)) {
            fds[STDOUT_FD].fd = -1;
        }
        if (fds[STDERR_FD].revents & (POLLERR | POLLHUP)) {
            fds[STDERR_FD].fd = -1;
        }
    }

    waitpid(child_pid, &child_exit_status, 0);

    int returncode;
    if (WIFEXITED(child_exit_status)) {
        returncode = WEXITSTATUS(child_exit_status);
    } else if (WIFSIGNALED(child_exit_status)) {
        returncode = -WTERMSIG(child_exit_status);
    } else {
        throw libdnf5::RuntimeError(M_("unexpected child process status after waitpid"));
    }

    return CompletedProcess{
        .returncode = returncode,
        .stdout = stdout_bytes,
        .stderr = stderr_bytes,
    };
}

}  // namespace libdnf5::utils
