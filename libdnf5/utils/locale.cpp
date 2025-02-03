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

#include "libdnf5/utils/locale.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"


namespace libdnf5::utils {

class Locale::Impl {
public:
    ~Impl() { freelocale(c_locale); }

private:
    explicit Impl(const char * std_name) : cpp_locale(std_name) {
        c_locale = newlocale(LC_ALL_MASK, std_name, (locale_t)0);
        if (c_locale == (locale_t)0) {
            throw std::runtime_error("newlocale failed");
        }
    }

    const std::locale & get_cpp_locale() const { return cpp_locale; }

    ::locale_t get_c_locale() const { return c_locale; }

    friend class Locale;

    std::locale cpp_locale;
    ::locale_t c_locale;
};


Locale::Locale(const char * std_name) {
    libdnf_assert(std_name != nullptr, "Locale name \"std_name\" contains null pointer");

    try {
        p_impl = ImplPtr(new Impl(std_name));
    } catch (const std::runtime_error & ex) {
        throw RuntimeError(M_("Cannot set the locale \"{}\": {}"), std::string(std_name), std::string(ex.what()));
    }
}

Locale::~Locale() = default;

const std::locale & Locale::get_cpp_locale() const {
    return p_impl->get_cpp_locale();
}

::locale_t Locale::get_c_locale() const {
    return p_impl->get_c_locale();
}

}  // namespace libdnf5::utils
