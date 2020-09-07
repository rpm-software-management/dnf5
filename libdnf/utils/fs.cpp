#include <sys/stat.h>

#include <filesystem>

#include "fs.hpp"


namespace libdnf::utils::fs {


void makedirs_for_file(const std::string & file_path) {
    // get an absolute path that may not exist
    auto path = std::filesystem::weakly_canonical(file_path);

    // remove the file name
    path = path.parent_path();

    // iterate the absolute path from the beginning and create the missing parts
    std::filesystem::path path_to_create;
    for (auto & part : path) {
        path_to_create /= part;
        if (!std::filesystem::exists(path_to_create)) {
            // TODO(dmach): check return value
            mkdir(path_to_create.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
    }
}


}  // namespace libdnf::utils::fs
