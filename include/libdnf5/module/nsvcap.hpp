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

#ifndef LIBDNF_MODULE_NSCVAP_HPP
#define LIBDNF_MODULE_NSCVAP_HPP

#include <string>
#include <vector>


namespace libdnf5::module {


class Nsvcap {
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

    /// The default forms and their order determine module_spec matching
    static const std::vector<Form> & get_default_module_spec_forms();

    /// Parse a string in a given form.
    /// If the parsing is successful, the attributes of this class are set accordingly, otherwise, they are left unchanged.
    ///
    /// @param pattern          A string to parse.
    /// @param form             A module form in which the string should be.
    /// @return                 `true` if the pattern matched the form, `false` otherwise.
    /// @since 5.0.6
    bool parse(const std::string pattern, Form form);

    /// Parse a string in one of the given forms.
    /// If the parsing is successful, the attributes of this class are set accordingly, otherwise, they are left unchanged.
    ///
    /// @param pattern          A string to parse.
    /// @param forms            Module forms in one of which the string should be.
    /// @return                 `true` if the nsvcap_str matched one of the forms, `false` otherwise.
    /// @since 5.0.6
    static std::vector<Nsvcap> parse(const std::string & pattern, std::vector<Form> forms);

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

    const std::string & get_name() const noexcept { return name; }
    const std::string & get_stream() const noexcept { return stream; }
    const std::string & get_version() const noexcept { return version; }
    const std::string & get_context() const noexcept { return context; }
    const std::string & get_arch() const noexcept { return arch; }
    const std::string & get_profile() const noexcept { return profile; }

    void set_name(const std::string & name) { this->name = name; }
    void set_stream(const std::string & stream) { this->stream = stream; }
    void set_version(const std::string & version) { this->version = version; }
    void set_context(const std::string & context) { this->context = context; }
    void set_arch(const std::string & arch) { this->arch = arch; }
    void set_profile(const std::string & profile) { this->profile = profile; }

    void set_name(std::string && name) { this->name = std::move(name); }
    void set_stream(std::string && stream) { this->stream = std::move(stream); }
    void set_version(std::string && version) { this->version = std::move(version); }
    void set_context(std::string && context) { this->context = std::move(context); }
    void set_arch(std::string && arch) { this->arch = std::move(arch); }
    void set_profile(std::string && profile) { this->profile = std::move(profile); }

private:
    std::string name;
    std::string stream;
    std::string version;
    std::string context;
    std::string arch;
    std::string profile;
};


}  // namespace libdnf5::module

#endif  //LIBDNF_MODULE_NSCVAP_HPP
