#include "fs.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <filesystem>

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

bool have_files_same_content_noexcept(const char * file_path1, const char * file_path2) noexcept {
    static constexpr int block_size = 4096;
    bool ret = false;
    int fd1 = -1;
    int fd2 = -1;
    do {
        if ((fd1 = open(file_path1, O_CLOEXEC)) == -1) {
            break;
        }
        if ((fd2 = open(file_path2, O_CLOEXEC)) == -1) {
            break;
        }
        auto len1 = lseek(fd1, 0, SEEK_END);
        auto len2 = lseek(fd2, 0, SEEK_END);
        if (len1 != len2) {
            break;
        }
        ret = true;
        if (len1 == 0) {
            break;
        }
        lseek(fd1, 0, SEEK_SET);
        lseek(fd2, 0, SEEK_SET);
        char buf1[block_size];
        char buf2[block_size];
        ssize_t readed;
        do {
            readed = read(fd1, &buf1, block_size);
            auto readed2 = read(fd2, &buf2, block_size);
            if (readed2 != readed || std::memcmp(&buf1, &buf2, static_cast<size_t>(readed)) != 0) {
                ret = false;
                break;
            }
        } while (readed == block_size);
    } while (false);

    if (fd1 != -1) {
        close(fd1);
    }
    if (fd2 != -1) {
        close(fd2);
    }
    return ret;
}


}  // namespace libdnf::utils::fs
