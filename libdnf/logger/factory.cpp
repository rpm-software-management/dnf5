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

#include "libdnf/logger/factory.hpp"

#include "libdnf/logger/stream_logger.hpp"

#include <filesystem>
#include <fstream>


namespace libdnf {

using namespace std::filesystem;

std::unique_ptr<libdnf::Logger> create_file_logger(Base & base) {
    auto & config = base.get_config();
    auto & installroot = config.get_installroot_option().get_value();
    auto & logdir = config.get_logdir_option().get_value();
    auto logdir_full_path = path(installroot) / path(logdir).relative_path();

    create_directories(logdir_full_path);
    auto log_file = logdir_full_path / FILE_LOGGER_FILENAME;
    auto log_stream = std::make_unique<std::ofstream>(log_file, std::ios::app);
    if (!log_stream->is_open()) {
        throw std::runtime_error(fmt::format("Cannot open log file: {}: {}", log_file.c_str(), strerror(errno)));
    }

    // Throw exceptions if there is an error while writing to the log stream
    log_stream->exceptions(std::ios::badbit | std::ios::failbit);

    return std::make_unique<libdnf::StreamLogger>(std::move(log_stream));
}

}  // namespace libdnf
