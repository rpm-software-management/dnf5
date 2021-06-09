/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_RPM_NEVRA_HPP
#define LIBDNF_RPM_NEVRA_HPP

#include "libdnf/common/exception.hpp"

#include <string>
#include <vector>

namespace libdnf::rpm {

/// @replaces hawkey:hawkey/__init__.py:class:Nevra
struct Nevra {
public:
    enum class Form { NEVRA = 1, NEVR = 2, NEV = 3, NA = 4, NAME = 5 };

    struct IncorrectNevraString : public RuntimeError {
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::rpm::Nevra"; }
        const char * get_name() const noexcept override { return "IncorrectNevraString"; }
        const char * get_description() const noexcept override { return "Nevra exception"; }
    };

    /// The default forms and their order determine pkg_spec matching
    static const std::vector<Form> & get_default_pkg_spec_forms();
    /// Parse string into Nevra struct
    /// @param - string to parse
    /// @return - Vector with parsed Nevra
    /// @exception - IncorrectNevraString
    /// @since - 1.0.0
    static std::vector<Nevra> parse(const std::string & nevra_str);
    static std::vector<Nevra> parse(const std::string & nevra_str, const std::vector<Form> & forms);

    Nevra() = default;
    Nevra(const Nevra & src) = default;
    Nevra(Nevra && src) = default;
    Nevra & operator=(const Nevra & other) = default;

    /// Returns false when parsing failed and stored data are in inconsistance state.

    void clear() noexcept;

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.name
    const std::string & get_name() const noexcept { return name; }

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.epoch
    const std::string & get_epoch() const noexcept { return epoch; }

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.version
    const std::string & get_version() const noexcept { return version; }

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.release
    const std::string & get_release() const noexcept { return release; }

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.arch
    const std::string & get_arch() const noexcept { return arch; }

    void set_name(const std::string & value) { name = value; }
    void set_epoch(const std::string & value) { epoch = value; }
    void set_version(const std::string & value) { version = value; }
    void set_release(const std::string & value) { release = value; }
    void set_arch(const std::string & value) { arch = value; }

    void set_name(const std::string && value) { name = std::move(value); }
    void set_epoch(const std::string && value) { epoch = std::move(value); }
    void set_version(const std::string && value) { version = std::move(value); }
    void set_release(const std::string && value) { release = std::move(value); }
    void set_arch(const std::string && value) { arch = std::move(value); }

    // TODO(jmracek) Add comperators ==

    /// @replaces hawkey:hawkey/__init__.py:method:Nevra.has_just_name()
    bool has_just_name() const;

private:
    std::string name;
    std::string epoch;
    std::string version;
    std::string release;
    std::string arch;
};


inline std::vector<Nevra> Nevra::parse(const std::string & nevra_str) {
    return parse(nevra_str, get_default_pkg_spec_forms());
}


/// Create a full nevra string (always contains epoch) from an object
template <typename T>
inline std::string to_full_nevra_string(const T & obj) {
    auto epoch = obj.get_epoch();
    if (epoch.empty()) {
        epoch = "0";
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


}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_NEVRA_HPP
