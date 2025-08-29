// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_COMMANDS_COPR_DOWNLOAD_FILE_HPP
#define DNF5_COMMANDS_COPR_DOWNLOAD_FILE_HPP

#include <libdnf5/base/base.hpp>

#include <filesystem>

void download_file(libdnf5::Base & base, const std::string & url, const std::filesystem::path & path);

#endif  // DNF5_COMMANDS_COPR_DOWNLOAD_FILE_HPP
