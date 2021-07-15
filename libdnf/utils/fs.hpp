#ifndef LIBDNF_UTILS_FS_HPP
#define LIBDNF_UTILS_FS_HPP

#include <string>


namespace libdnf::utils::fs {

/// Create directories for a specified path to a file.
/// Make sure trailing '/' is present if a file name is not specified.
void makedirs_for_file(const std::string & file_path);

/// Compares content of two files.
/// Returns "true" if files have the same content.
/// If content differs or error occurred (file doesn't exist, not readable, ...) returns "false".
bool have_files_same_content_noexcept(const char * file_path1, const char * file_path2) noexcept;

}  // namespace libdnf::utils::fs

#endif  // LIBDNF_UTILS_FS_HPP
