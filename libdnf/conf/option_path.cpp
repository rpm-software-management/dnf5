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

#include "libdnf/conf/option_path.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace libdnf {

static std::string remove_file_prot(const std::string & value) {
    const int prefix_len = 7;
    if (value.compare(0, prefix_len, "file://") == 0) {
        return value.substr(prefix_len);
    }
    return value;
}

OptionPath::OptionPath(const std::string & default_value, bool exists, bool abs_path)
    : OptionString(default_value)
    , exists(exists)
    , abs_path(abs_path) {
    this->default_value = remove_file_prot(this->default_value);
    test(this->default_value);
    this->value = this->default_value;
}

OptionPath::OptionPath(const char * default_value, bool exists, bool abs_path)
    : OptionString(default_value)
    , exists(exists)
    , abs_path(abs_path) {
    if (default_value) {
        this->default_value = remove_file_prot(this->default_value);
        test(this->default_value);
        this->value = this->default_value;
    }
}

OptionPath::OptionPath(
    const std::string & default_value, const std::string & regex, bool icase, bool exists, bool abs_path)
    : OptionString(remove_file_prot(default_value), regex, icase)
    , exists(exists)
    , abs_path(abs_path) {
    this->default_value = remove_file_prot(this->default_value);
    test(this->default_value);
    this->value = this->default_value;
}

OptionPath::OptionPath(const char * default_value, const std::string & regex, bool icase, bool exists, bool abs_path)
    : OptionString(default_value, regex, icase)
    , exists(exists)
    , abs_path(abs_path) {
    if (default_value) {
        this->default_value = remove_file_prot(this->default_value);
        test(this->default_value);
        this->value = this->default_value;
    }
}

void OptionPath::test(const std::string & value) const {
    if (abs_path && value[0] != '/') {
        throw NotAllowedValue(value);
    }

    struct stat buffer;
    if (exists && stat(value.c_str(), &buffer) != 0) {
        throw PathNotExists(value);
    }
}

void OptionPath::set(Priority priority, const std::string & value) {
    if (is_locked()) {
        throw WriteLocked(get_lock_comment());
    }
    if (priority >= get_priority()) {
        OptionString::test(value);
        auto val = remove_file_prot(value);
        test(val);
        this->value = val;
        set_priority(priority);
    }
}

}  // namespace libdnf
