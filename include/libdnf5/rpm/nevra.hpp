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


#ifndef LIBDNF5_RPM_NEVRA_HPP
#define LIBDNF5_RPM_NEVRA_HPP

#include "nevra_errors.hpp"

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <sstream>
#include <string>
#include <vector>


namespace libdnf5::rpm {

// @replaces hawkey:hawkey/__init__.py:class:Nevra
struct LIBDNF_API Nevra {
public:
    enum class Form { NEVRA = 1, NEVR = 2, NEV = 3, NA = 4, NAME = 5 };

    /// The default forms and their order determine pkg_spec matching
    static const std::vector<Form> & get_default_pkg_spec_forms();

    /// Parse string into Nevra struct
    /// @param nevra_str String to parse
    /// @return Vector with parsed Nevra
    /// @exception IncorrectNevraString
    /// @since - 1.0.0
    static std::vector<Nevra> parse(const std::string & nevra_str);

    /// Parse string into Nevra struct using given forms
    /// @param nevra_str String to parse
    /// @param forms Allowed forms used for parsing
    /// @return Vector with parsed Nevra
    /// @exception IncorrectNevraString
    static std::vector<Nevra> parse(const std::string & nevra_str, const std::vector<Form> & forms);

    Nevra();
    ~Nevra();
    Nevra(const Nevra & src);
    Nevra(Nevra && src) noexcept;
    Nevra & operator=(const Nevra & other);
    Nevra & operator=(Nevra && src) noexcept;

    /// @return `true` if all Nevra attributes (`name`, `epoch`, `version`, `release` and `arch`) match.
    bool operator==(const Nevra & other) const;

    /// Compares Nevra attributes in order: `name`, `epoch`, `version`, `release`, `arch`, and checks
    /// if any is less than the corresponding attribute.
    /// @return `true` if any of the attributes is less than the corresponding one. Return `false` otherwise.
    bool operator<(const Nevra & other) const;

    bool operator>(const Nevra & other) const { return other < *this; }
    bool operator<=(const Nevra & other) const { return !(*this > other); }
    bool operator>=(const Nevra & other) const { return !(*this < other); }

    // NOTE: required by cppunit asserts
    //friend std::ostringstream & operator<<(std::ostringstream & out, const Nevra & nevra);

    /// Returns false when parsing failed and stored data are in inconsistency state.

    void clear() noexcept;

    // @replaces hawkey:hawkey/__init__.py:attribute:Nevra.name
    const std::string & get_name() const noexcept;

    // @replaces hawkey:hawkey/__init__.py:attribute:Nevra.epoch
    const std::string & get_epoch() const noexcept;

    // @replaces hawkey:hawkey/__init__.py:attribute:Nevra.version
    const std::string & get_version() const noexcept;

    // @replaces hawkey:hawkey/__init__.py:attribute:Nevra.release
    const std::string & get_release() const noexcept;

    // @replaces hawkey:hawkey/__init__.py:attribute:Nevra.arch
    const std::string & get_arch() const noexcept;

    void set_name(const std::string & value);
    void set_epoch(const std::string & value);
    void set_version(const std::string & value);
    void set_release(const std::string & value);
    void set_arch(const std::string & value);

    void set_name(std::string && value);
    void set_epoch(std::string && value);
    void set_version(std::string && value);
    void set_release(std::string && value);
    void set_arch(std::string && value);

    // @replaces hawkey:hawkey/__init__.py:method:Nevra.has_just_name()
    bool has_just_name() const;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


inline std::vector<Nevra> Nevra::parse(const std::string & nevra_str) {
    return parse(nevra_str, get_default_pkg_spec_forms());
}


LIBDNF_API std::ostringstream & operator<<(std::ostringstream & out, const Nevra & nevra);


/// Create a full nevra string (always contains epoch) from an object
template <typename T>
inline std::string to_full_nevra_string(const T & obj) {
    auto epoch = obj.get_epoch();
    if (epoch.empty()) {
// TODO(lukash) gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=104336
//                       https://bugzilla.redhat.com/show_bug.cgi?id=2057597
// Remove the pragmas when fixed
#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
#endif
        epoch = "0";
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
    }
    // reserve() & append() is about 25% faster than string concatenation
    std::string result;
    result.reserve(
        4 + obj.get_name().size() + epoch.size() + obj.get_version().size() + obj.get_release().size() +
        obj.get_arch().size());
    result.append(obj.get_name());
    result.append("-");
    result.append(epoch);
    result.append(":");
    result.append(obj.get_version());
    result.append("-");
    result.append(obj.get_release());
    result.append(".");
    result.append(obj.get_arch());
    return result;
}


/// Create a nevra string (0 epoch excluded) from an object
template <typename T>
inline std::string to_nevra_string(const T & obj) {
    auto epoch = obj.get_epoch();
    if (epoch.empty() || epoch == "0") {
        // reserve() & append() is about 25% faster than string concatenation
        std::string result;
        result.reserve(
            3 + obj.get_name().size() + obj.get_version().size() + obj.get_release().size() + obj.get_arch().size());
        result.append(obj.get_name());
        result.append("-");
        result.append(obj.get_version());
        result.append("-");
        result.append(obj.get_release());
        result.append(".");
        result.append(obj.get_arch());
        return result;
    }
    return to_full_nevra_string(obj);
}


/// Copy nevra attributes from one object to another
template <typename F, typename T>
inline void copy_nevra_attributes(const F & from, T & to) {
    to.set_name(from.get_name());
    to.set_epoch(from.get_epoch());
    to.set_version(from.get_version());
    to.set_release(from.get_release());
    to.set_arch(from.get_arch());
}


/// Compare alpha and numeric segments of two versions.
/// @return 1 if `lhs` > `rhs`, -1 if `lhs` < `rhs`, 0 if they are equal
LIBDNF_API int rpmvercmp(const char * lhs, const char * rhs);


/// Compare evr part of two objects
/// @return 1 if `lhs` > `rhs`, -1 if `lhs` < `rhs`, 0 if they are equal
template <typename L, typename R>
int evrcmp(const L & lhs, const R & rhs) {
    // handle empty epoch the same way as 0 epoch
    auto lepoch = lhs.get_epoch();
    auto repoch = rhs.get_epoch();
    int r = rpmvercmp(lepoch.empty() ? "0" : lepoch.c_str(), repoch.empty() ? "0" : repoch.c_str());
    if (r != 0) {
        return r;
    }

    r = rpmvercmp(lhs.get_version().c_str(), rhs.get_version().c_str());
    if (r != 0) {
        return r;
    }

    return rpmvercmp(lhs.get_release().c_str(), rhs.get_release().c_str());
}

/// Compare two objects by their Name, Epoch:Version-Release and Arch.
/// @return `true` if `lhs` < `rhs`. Return `false` otherwise.
template <typename L, typename R>
bool cmp_nevra(const L & lhs, const R & rhs) {
    // compare by name
    int r = lhs.get_name().compare(rhs.get_name());
    if (r < 0) {
        return true;
    }
    if (r > 0) {
        return false;
    }

    // names are equal, compare by evr
    r = evrcmp(lhs, rhs);
    if (r < 0) {
        return true;
    }
    if (r > 0) {
        return false;
    }

    // names and evrs are equal, compare by arch
    r = lhs.get_arch().compare(rhs.get_arch());
    return r < 0;
};

template <typename T>
bool cmp_nevra(const T & lhs, const T & rhs) {
    return cmp_nevra<T, T>(lhs, rhs);
}

/// Compare two objects by their Name, Arch and Epoch:Version-Release.
/// @return `true` if `lhs` < `rhs`. Return `false` otherwise.
template <typename L, typename R>
bool cmp_naevr(const L & lhs, const R & rhs) {
    // compare by name
    int r = lhs.get_name().compare(rhs.get_name());
    if (r < 0) {
        return true;
    }
    if (r > 0) {
        return false;
    }

    // names are equal, compare by arch
    r = lhs.get_arch().compare(rhs.get_arch());
    if (r < 0) {
        return true;
    }
    if (r > 0) {
        return false;
    }

    // names and arches are equal, compare by evr
    r = evrcmp(lhs, rhs);
    return r < 0;
};

template <typename T>
bool cmp_naevr(const T & lhs, const T & rhs) {
    return cmp_naevr<T, T>(lhs, rhs);
}

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_NEVRA_HPP
