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

#include <string>
#include <vector>

namespace libdnf::rpm {

/// @replaces hawkey:hawkey/__init__.py:class:Nevra
struct Nevra {
public:
    enum class Form { NEVRA = 1, NEVR = 2, NEV = 3, NA = 4, NAME = 5 };

    /// The forms and their order determine pkg_spec matching
    static const std::vector<Form> PKG_SPEC_FORMS;

    Nevra() = default;
    Nevra(const Nevra & src) = default;
    Nevra(Nevra && src) = default;

    /// Returns false when parsing failed and stored data are in inconsistance state.
    bool parse(const std::string & nevra_str, Form form);
    void clear() noexcept;

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.name
    const std::string & get_name() const noexcept;

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.epoch
    const std::string & get_epoch() const noexcept;

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.version
    const std::string & get_version() const noexcept;

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.release
    const std::string & get_release() const noexcept;

    /// @replaces hawkey:hawkey/__init__.py:attribute:Nevra.arch
    const std::string & get_arch() const noexcept;

    void set_name(const std::string & name);
    void set_epoch(const std::string & epoch);
    void set_version(const std::string & version);
    void set_release(const std::string & release);
    void set_arch(const std::string & arch);

    void set_name(std::string && name);
    void set_epoch(std::string && epoch);
    void set_version(std::string && version);
    void set_release(std::string && release);
    void set_arch(std::string && arch);

    std::string get_evr() const;

    // TODO(jmracek) Shall ve ass get_evr method?
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

inline const std::vector<Nevra::Form> Nevra::PKG_SPEC_FORMS(
    {Nevra::Form::NEVRA, Nevra::Form::NA, Nevra::Form::NAME, Nevra::Form::NEVR, Nevra::Form::NEV});

inline const std::string & Nevra::get_name() const noexcept {
    return name;
}

inline const std::string & Nevra::get_epoch() const noexcept {
    return epoch;
}

inline const std::string & Nevra::get_version() const noexcept {
    return version;
}

inline const std::string & Nevra::get_release() const noexcept {
    return release;
}

inline const std::string & Nevra::get_arch() const noexcept {
    return arch;
}

inline void Nevra::set_name(const std::string & name) {
    this->name = name;
}

inline void Nevra::set_epoch(const std::string & epoch) {
    this->epoch = epoch;
}

inline void Nevra::set_version(const std::string & version) {
    this->version = version;
}

inline void Nevra::set_release(const std::string & release) {
    this->release = release;
}

inline void Nevra::set_arch(const std::string & arch) {
    this->arch = arch;
}

inline void Nevra::set_name(std::string && name) {
    this->name = std::move(name);
}

inline void Nevra::set_epoch(std::string && epoch) {
    this->epoch = std::move(epoch);
}

inline void Nevra::set_version(std::string && version) {
    this->version = std::move(version);
}

inline void Nevra::set_release(std::string && release) {
    this->release = std::move(release);
}

inline void Nevra::set_arch(std::string && arch) {
    this->arch = std::move(arch);
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
    result.reserve(4 + obj.get_name().size() + epoch.size() + obj.get_version().size() + obj.get_release().size() + obj.get_arch().size());
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
        result.reserve(3 + obj.get_name().size() + obj.get_version().size() + obj.get_release().size() + obj.get_arch().size());
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
