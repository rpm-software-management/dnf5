#ifndef LIBDNF_UTILS_STRING_HPP
#define LIBDNF_UTILS_STRING_HPP


#include <algorithm>
#include <string>


namespace libdnf::utils::string {


/// Determine if a string starts with a pattern
inline bool starts_with(const std::string & value, const std::string & pattern) {
    if (pattern.size() > value.size()) {
        return false;
    }
    return std::equal(pattern.begin(), pattern.end(), value.begin());
}


/// Determine if a string ends with a pattern
inline bool ends_with(const std::string & value, const std::string & pattern) {
    if (pattern.size() > value.size()) {
        return false;
    }
    return std::equal(pattern.rbegin(), pattern.rend(), value.rbegin());
}


}  // libdnf::utils::string


#endif  // LIBDNF_UTILS_STRING_HPP
