#ifndef LIBDNF_UTILS_FS_HPP
#define LIBDNF_UTILS_FS_HPP


#include <string>


namespace libdnf::utils::fs {


/// Create directories for a specified path to a file.
/// Make sure trailing '/' is present if a file name is not specified.
void makedirs_for_file(const std::string & file_path);


}  // namespace libdnf::utils::fs


#endif  // LIBDNF_UTILS_FS_HPP
