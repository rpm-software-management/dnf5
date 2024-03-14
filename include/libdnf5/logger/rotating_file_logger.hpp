/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF5_LOGGER_ROTATING_FILE_LOGGER_HPP
#define LIBDNF5_LOGGER_ROTATING_FILE_LOGGER_HPP

#include "logger.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <filesystem>

namespace libdnf5 {

/// RotatingFileLogger is an implementation of a rotating file logger.
/// It can be used simultaneously in multiple processes and threads.
class LIBDNF_API RotatingFileLogger : public StringLogger {
public:
    /// Construct a new instance of the `RotatingFileLogger` class.
    ///
    /// It immediately opens (creates) `base_file_path` to detect problems before logging starts.
    ///
    /// When `max_bytes` is non-zero and `backup_count` is non-zero, rotation is enabled.
    /// For example, with `backup_count` of 5 and a `base_file_path` of "app.log", we would get "app.log", "app.log.1",
    /// "app.log.2" through "app.log.5". The file written to is always "app.log". If the message being written does not
    /// fit into the file (the size of the resulting file would be greater than `max_bytes`), it is closed and renamed
    /// to "app.log.1", and if there are files "app.log.1", "app.log.2", etc., then they are renamed on "app.log.2",
    /// "app.log.3" etc. Writing is done to a new "app.log" file.
    /// Note: The message being written is not split into multiple files. -> The log file can be larger than `max_bytes`
    /// if the message being written is larger than `max_bytes`. The file then only contains this message.
    ///
    /// @param base_file_path path to the file where log messages are written
    /// @param max_bytes      max log file size; 0 - means unlimited; at least one full message can always be written
    /// @param backup_count   maximum number of backup files; 0 - means rotation is disabled
    explicit RotatingFileLogger(
        const std::filesystem::path & base_file_path, std::size_t max_bytes, std::size_t backup_count);

    ~RotatingFileLogger();

    using StringLogger::write;

    void write(const char * line) noexcept override;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
