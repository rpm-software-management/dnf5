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

#include "libdnf5/conf/option_path.hpp"

#include "option_string_private.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace libdnf5 {

class OptionPath::Impl {
public:
    Impl(bool exists, bool abs_path) : exists(exists), abs_path(abs_path) {};

private:
    friend OptionPath;

    bool exists;
    bool abs_path;
};

static std::string remove_file_prot(const std::string & value) {
    const int prefix_len = 7;
    if (value.compare(0, prefix_len, "file://") == 0) {
        return value.substr(prefix_len);
    }
    return value;
}

OptionPath::OptionPath(std::string default_value, bool exists, bool abs_path)
    : OptionString(default_value),
      p_impl(new Impl(exists, abs_path)) {
    OptionString::p_impl->default_value = remove_file_prot(this->get_default_value());
    test(this->get_default_value());
    OptionString::p_impl->value = this->get_default_value();
}

OptionPath::OptionPath(const char * default_value, bool exists, bool abs_path)
    : OptionString(default_value),
      p_impl(new Impl(exists, abs_path)) {
    if (default_value) {
        OptionString::p_impl->default_value = remove_file_prot(this->get_default_value());
        test(this->get_default_value());
        OptionString::p_impl->value = this->get_default_value();
    }
}

OptionPath::OptionPath(std::string default_value, std::string regex, bool icase, bool exists, bool abs_path)
    : OptionString(remove_file_prot(default_value), regex, icase),
      p_impl(new Impl(exists, abs_path)) {
    OptionString::p_impl->default_value = remove_file_prot(this->get_default_value());
    test(this->get_default_value());
    OptionString::p_impl->value = this->get_default_value();
}

OptionPath::OptionPath(const char * default_value, std::string regex, bool icase, bool exists, bool abs_path)
    : OptionString(default_value, regex, icase),
      p_impl(new Impl(exists, abs_path)) {
    if (default_value) {
        OptionString::p_impl->default_value = remove_file_prot(this->get_default_value());
        test(this->get_default_value());
        OptionString::p_impl->value = this->get_default_value();
    }
}

OptionPath::~OptionPath() = default;

OptionPath::OptionPath(const OptionPath & src) = default;

OptionPath & OptionPath::operator=(const OptionPath & other) = default;

void OptionPath::test(const std::string & value) const {
    if (p_impl->abs_path && value[0] != '/') {
        throw OptionValueNotAllowedError(M_("Only absolute paths allowed, relative path \"{}\" detected"), value);
    }

    struct stat buffer;
    if (p_impl->exists && stat(value.c_str(), &buffer) != 0) {
        throw OptionPathNotFoundError(M_("Path \"{}\" does not exist"), value);
    }
}

void OptionPath::set(Priority priority, const std::string & value) {
    assert_not_locked();

    if (priority >= get_priority()) {
        OptionString::test(value);
        auto val = remove_file_prot(value);
        test(val);
        OptionString::p_impl->value = val;
        set_priority(priority);
    }
}

void OptionPath::set(const std::string & value) {
    set(Priority::RUNTIME, value);
}

OptionPath * OptionPath::OptionPath::clone() const {
    return new OptionPath(*this);
}


}  // namespace libdnf5
