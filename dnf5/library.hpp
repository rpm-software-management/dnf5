// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_UTILS_LIBRARY_HPP
#define DNF5_UTILS_LIBRARY_HPP

#include <string>


namespace dnf5::utils {

class Library {
public:
    explicit Library(const std::string & path);
    ~Library();

    Library(const Library &) = delete;
    Library(Library &&) = delete;
    Library & operator=(const Library &) = delete;
    Library & operator=(Library &&) = delete;

    const std::string & get_path() const noexcept { return path; }
    void * get_address(const char * symbol) const;

private:
    std::string path;
    void * handle;
};

}  // namespace dnf5::utils

#endif
