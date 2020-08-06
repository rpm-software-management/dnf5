#ifndef LIBDNF_CLI_UTILS_UTF8
#define LIBDNF_CLI_UTILS_UTF8


#include <string>


namespace libdnf::cli::utils::utf8 {


/// return length of an utf-8 encoded string
std::size_t length(const std::string & str);


/// return printable width of an utf-8 encoded string (considers non-printable and wide characters)
std::size_t width(const std::string & str);


/// return an utf-8 sub-string that matches specified character count
std::string substr_length(const std::string & str, std::string::size_type pos = 0, std::string::size_type len = std::string::npos);


/// return an utf-8 sub-string that matches specified printable width
std::string substr_width(const std::string & str, std::string::size_type pos = 0, std::string::size_type wid = std::string::npos);


}  // namespace libdnf::cli::utils::utf8


#endif  // LIBDNF_CLI_UTILS_UTF8
