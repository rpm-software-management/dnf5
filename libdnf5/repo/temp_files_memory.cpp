// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "temp_files_memory.hpp"

#include "utils/string.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/file.hpp"

#include <toml.hpp>


namespace libdnf5::repo {

TempFilesMemory::TempFilesMemory(const BaseWeakPtr & base, const std::string & parent_dir)
    : base(base),
      full_memory_path(std::filesystem::path(parent_dir) / MEMORY_FILENAME) {
    std::filesystem::create_directories(parent_dir);
}

TempFilesMemory::~TempFilesMemory() = default;

std::vector<std::string> TempFilesMemory::get_files() const {
    if (!std::filesystem::exists(full_memory_path)) {
        return {};
    }

    try {
        auto toml_data = toml::parse(full_memory_path);
        return toml::find_or(toml_data, FILE_PATHS_TOML_KEY, std::vector<std::string>{});
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
    auto new_data = toml::format(toml::value(toml::table{{FILE_PATHS_TOML_KEY, files}}));
    // Although it's not clear for the documentation if it is possible,
    // it occurred, that the file was empty, which results in parsing error,
    // see https://github.com/rpm-software-management/dnf5/issues/1001
    if (new_data.empty()) {
        auto & logger = *base->get_logger();
        logger.error(
            "Serializing temporary file paths failed for the input vector: \"{}\"", utils::string::join(files, ", "));
        return;
    }
    utils::fs::File(temporary_path, "w").write(new_data);
    std::filesystem::rename(temporary_path, full_memory_path);
}

void TempFilesMemory::clear() {
    std::filesystem::remove(full_memory_path);
}

}  // namespace libdnf5::repo
