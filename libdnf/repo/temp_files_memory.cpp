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

#include "temp_files_memory.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/fs/file.hpp"

#include "libdnf/common/exception.hpp"

#include <toml.hpp>


namespace libdnf5::repo {

TempFilesMemory::TempFilesMemory(const std::string & parent_dir)
    : full_memory_path(std::filesystem::path(parent_dir) / MEMORY_FILENAME) {
    std::filesystem::create_directories(parent_dir);
}

TempFilesMemory::~TempFilesMemory() = default;

std::vector<std::string> TempFilesMemory::get_files() const {
    if (!std::filesystem::exists(full_memory_path)) {
        return {};
    }

    try {
        auto toml_data = toml::parse(full_memory_path);
        return toml::get<std::vector<std::string>>(toml_data[FILE_PATHS_TOML_KEY]);
    } catch (const toml::exception & e) {
        throw libdnf5::Error(
            M_("An error occurred when parsing the temporary files memory file at '{}': {}"),
            full_memory_path.string(),
            std::string(e.what()));
    }
}

void TempFilesMemory::add_files(const std::vector<std::string> & paths) {
    auto files = get_files();
    files.insert(files.end(), paths.begin(), paths.end());

    // sort and deduplicate resulting vector
    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());

    // write new contents to a temporary file and then move the new file atomically
    auto temporary_path = full_memory_path.string() + ".temp";
    utils::fs::File(temporary_path, "w").write(toml::format(toml::value({{FILE_PATHS_TOML_KEY, files}})));
    std::filesystem::rename(temporary_path, full_memory_path);
}

void TempFilesMemory::clear() {
    std::filesystem::remove(full_memory_path);
}

}  // namespace libdnf5::repo
