// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_MODULE_NSCVAP_HPP
#define LIBDNF5_MODULE_NSCVAP_HPP

#include "libdnf5/defs.h"

#include <memory>
#include <string>
#include <vector>


namespace libdnf5::module {


class LIBDNF_API Nsvcap {
public:
    enum class Form {
        NSVCAP = 1,
        NSVCA = 2,
        NSVAP = 3,
        NSVA = 4,
        NSAP = 5,
        NSA = 6,
        NSVCP = 7,
        NSVP = 8,
        NSVC = 9,
        NSV = 10,
        NSP = 11,
        NS = 12,
        NAP = 13,
        NA = 14,
        NP = 15,
        N = 16,
        NSCP = 17,
        NSC = 18,
    };

    Nsvcap();
    ~Nsvcap();

    Nsvcap(const Nsvcap & src);
    Nsvcap & operator=(const Nsvcap & src);

    Nsvcap(Nsvcap && src) noexcept;
    Nsvcap & operator=(Nsvcap && src) noexcept;

    /// The default forms and their order determine module_spec matching
    static const std::vector<Form> & get_default_module_spec_forms();

    /// Parse a string in a given form.
    /// If the parsing is successful, the attributes of this class are set accordingly, otherwise, they are left unchanged.
    ///
    /// @param nsvcap_str       A string to parse.
    /// @param form             A module form in which the string should be.
    /// @return                 `true` if the pattern matched the form, `false` otherwise.
    /// @since 5.0.6
    bool parse(const std::string & nsvcap_str, Form form);

    /// Parse a string in one of the given forms.
    /// If the parsing is successful, the attributes of this class are set accordingly, otherwise, they are left unchanged.
    ///
    /// @param pattern          A string to parse.
    /// @param forms            Module forms in one of which the string should be.
    /// @return                 `true` if the nsvcap_str matched one of the forms, `false` otherwise.
    /// @since 5.0.6
    static std::vector<Nsvcap> parse(const std::string & pattern, const std::vector<Form> & forms);

    /// Parse a string.
    /// If the parsing is successful, the attributes of this class are set accordingly, otherwise, they are left unchanged.
    ///
    /// @param pattern          A string to parse.
    /// @return                 `true` if the nsvcap_str matched any form, `false` otherwise.
    /// @since 5.0.6
    static std::vector<Nsvcap> parse(const std::string & pattern);

    /// Clear the attributes of this class - set them to empty strings.
    ///
    /// @since 5.0.6
    void clear();

    const std::string & get_name() const noexcept;
    const std::string & get_stream() const noexcept;
    const std::string & get_version() const noexcept;
    const std::string & get_context() const noexcept;
    const std::string & get_arch() const noexcept;
    const std::string & get_profile() const noexcept;

    void set_name(const std::string & name);
    void set_stream(const std::string & stream);
    void set_version(const std::string & version);
    void set_context(const std::string & context);
    void set_arch(const std::string & arch);
    void set_profile(const std::string & profile);

    void set_name(std::string && name);
    void set_stream(std::string && stream);
    void set_version(std::string && version);
    void set_context(std::string && context);
    void set_arch(std::string && arch);
    void set_profile(std::string && profile);

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::module

#endif  //LIBDNF5_MODULE_NSCVAP_HPP
