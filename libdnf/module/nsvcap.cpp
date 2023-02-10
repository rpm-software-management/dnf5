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

// TODO(pkratoch): Similar regexes are also defined in libdnf/conf/vars.cpp. Define them only once.
#define LETTERS        "a-zA-Z"
#define DIGITS         "0-9"
#define MODULE_SPECIAL "+._\\-"
#define GLOB           "][*?!"

#include "libdnf/module/nsvcap.hpp"

#include "utils/regex.hpp"

#include <array>
#include <regex>
#include <string>
#include <vector>


namespace libdnf::module {


#define MODULE_NAME    "([" GLOB LETTERS DIGITS MODULE_SPECIAL "]+)"
#define MODULE_STREAM  MODULE_NAME
#define MODULE_VERSION "([" GLOB DIGITS "\\-]+)"
#define MODULE_CONTEXT MODULE_NAME
#define MODULE_ARCH    MODULE_NAME
#define MODULE_PROFILE MODULE_NAME


// clang-format off
static const std::array<std::regex, 18> NSVCAP_FORM_REGEX{
    std::regex("^" MODULE_NAME ":" MODULE_STREAM ":" MODULE_VERSION ":" MODULE_CONTEXT "::?" MODULE_ARCH "\\/"  MODULE_PROFILE "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM ":" MODULE_VERSION ":" MODULE_CONTEXT "::?" MODULE_ARCH "\\/?" "()"           "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM ":" MODULE_VERSION     "()"           "::"  MODULE_ARCH "\\/"  MODULE_PROFILE "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM ":" MODULE_VERSION     "()"           "::"  MODULE_ARCH "\\/?" "()"           "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM     "()"               "()"           "::"  MODULE_ARCH "\\/"  MODULE_PROFILE "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM     "()"               "()"           "::"  MODULE_ARCH "\\/?" "()"           "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM ":" MODULE_VERSION ":" MODULE_CONTEXT       "()"        "\\/"  MODULE_PROFILE "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM ":" MODULE_VERSION     "()"                 "()"        "\\/"  MODULE_PROFILE "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM ":" MODULE_VERSION ":" MODULE_CONTEXT       "()"        "\\/?" "()"           "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM ":" MODULE_VERSION     "()"                 "()"        "\\/?" "()"           "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM     "()"               "()"                 "()"        "\\/"  MODULE_PROFILE "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM     "()"               "()"                 "()"        "\\/?" "()"           "$", std::regex::extended),
    std::regex("^" MODULE_NAME     "()"              "()"               "()"           "::"  MODULE_ARCH "\\/"  MODULE_PROFILE "$", std::regex::extended),
    std::regex("^" MODULE_NAME     "()"              "()"               "()"           "::"  MODULE_ARCH "\\/?" "()"           "$", std::regex::extended),
    std::regex("^" MODULE_NAME     "()"              "()"               "()"                 "()"        "\\/"  MODULE_PROFILE "$", std::regex::extended),
    std::regex("^" MODULE_NAME     "()"              "()"               "()"                 "()"        "\\/?" "()"           "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM     "()"           ":" MODULE_CONTEXT       "()"        "\\/?" MODULE_PROFILE "$", std::regex::extended),
    std::regex("^" MODULE_NAME ":" MODULE_STREAM     "()"           ":" MODULE_CONTEXT       "()"        "\\/?" "()"           "$", std::regex::extended)};
// clang-format on


static const std::vector<Nsvcap::Form> MODULE_SPEC_FORMS{
    Nsvcap::Form::N,
    Nsvcap::Form::NP,
    Nsvcap::Form::NS,
    Nsvcap::Form::NSP,
    Nsvcap::Form::NSC,
    Nsvcap::Form::NSCP,
    Nsvcap::Form::NSV,
    Nsvcap::Form::NSVP,
    Nsvcap::Form::NSVC,
    Nsvcap::Form::NSVCP,
    Nsvcap::Form::NA,
    Nsvcap::Form::NAP,
    Nsvcap::Form::NSA,
    Nsvcap::Form::NSAP,
    Nsvcap::Form::NSVA,
    Nsvcap::Form::NSVAP,
    Nsvcap::Form::NSVCA,
    Nsvcap::Form::NSVCAP};


const std::vector<Nsvcap::Form> & Nsvcap::get_default_module_spec_forms() {
    return MODULE_SPEC_FORMS;
}


bool Nsvcap::parse(const std::string nsvcap_str, Nsvcap::Form form) {
    enum { NAME = 1, STREAM = 2, VERSION = 3, CONTEXT = 4, ARCH = 5, PROFILE = 6, _LAST_ };

    if (nsvcap_str.length() > MAX_STRING_LENGTH_FOR_REGEX_MATCH) {
        // GCC std::regex_match() exhausts a stack on very long strings.
        return false;
    }

    std::smatch matched_result;
    auto matched = std::regex_match(nsvcap_str, matched_result, NSVCAP_FORM_REGEX[static_cast<unsigned>(form) - 1]);
    if (!matched || matched_result[NAME].str().size() == 0) {
        return false;
    }
    name = matched_result[NAME].str();
    version = matched_result[VERSION].str();
    stream = matched_result[STREAM].str();
    context = matched_result[CONTEXT].str();
    arch = matched_result[ARCH].str();
    profile = matched_result[PROFILE].str();
    return true;
}


std::vector<Nsvcap> Nsvcap::parse(const std::string & pattern, std::vector<Nsvcap::Form> forms) {
    std::vector<Nsvcap> nsvcaps;

    Nsvcap nsvcap;
    for (const auto & form : forms) {
        if (nsvcap.parse(pattern, form)) {
            nsvcaps.push_back(nsvcap);
        }
    }

    return nsvcaps;
}


std::vector<Nsvcap> Nsvcap::parse(const std::string & pattern) {
    return parse(pattern, MODULE_SPEC_FORMS);
}


void Nsvcap::clear() {
    name.clear();
    stream.clear();
    version.clear();
    context.clear();
    arch.clear();
    profile.clear();
}


}  // namespace libdnf::module
