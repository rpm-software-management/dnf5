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

// TODO(pkratoch): Similar regexes are also defined in libdnf5/conf/vars.cpp. Define them only once.
#define LETTERS        "a-zA-Z"
#define DIGITS         "0-9"
#define MODULE_SPECIAL "+._\\-"
#define GLOB           "][*?!"

#include "libdnf5/module/nsvcap.hpp"

#include "utils/regex.hpp"

#include <array>
#include <regex>
#include <string>
#include <vector>


namespace libdnf5::module {


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

class Nsvcap::Impl {
private:
    friend Nsvcap;

    std::string name;
    std::string stream;
    std::string version;
    std::string context;
    std::string arch;
    std::string profile;
};

Nsvcap::Nsvcap() : p_impl(std::make_unique<Impl>()) {}

Nsvcap::~Nsvcap() = default;

Nsvcap::Nsvcap(const Nsvcap & src) : p_impl(new Impl(*src.p_impl)) {}
Nsvcap::Nsvcap(Nsvcap && src) noexcept = default;

Nsvcap & Nsvcap::operator=(const Nsvcap & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
Nsvcap & Nsvcap::operator=(Nsvcap && src) noexcept = default;

const std::vector<Nsvcap::Form> & Nsvcap::get_default_module_spec_forms() {
    return MODULE_SPEC_FORMS;
}


bool Nsvcap::parse(const std::string & nsvcap_str, Nsvcap::Form form) {
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
    p_impl->name = matched_result[NAME].str();
    p_impl->version = matched_result[VERSION].str();
    p_impl->stream = matched_result[STREAM].str();
    p_impl->context = matched_result[CONTEXT].str();
    p_impl->arch = matched_result[ARCH].str();
    p_impl->profile = matched_result[PROFILE].str();
    return true;
}


std::vector<Nsvcap> Nsvcap::parse(const std::string & pattern, const std::vector<Nsvcap::Form> & forms) {
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
    p_impl->name.clear();
    p_impl->stream.clear();
    p_impl->version.clear();
    p_impl->context.clear();
    p_impl->arch.clear();
    p_impl->profile.clear();
}

const std::string & Nsvcap::get_name() const noexcept {
    return p_impl->name;
}
const std::string & Nsvcap::get_stream() const noexcept {
    return p_impl->stream;
}
const std::string & Nsvcap::get_version() const noexcept {
    return p_impl->version;
}
const std::string & Nsvcap::get_context() const noexcept {
    return p_impl->context;
}
const std::string & Nsvcap::get_arch() const noexcept {
    return p_impl->arch;
}
const std::string & Nsvcap::get_profile() const noexcept {
    return p_impl->profile;
}

void Nsvcap::set_name(const std::string & name) {
    p_impl->name = name;
}
void Nsvcap::set_stream(const std::string & stream) {
    p_impl->stream = stream;
}
void Nsvcap::set_version(const std::string & version) {
    p_impl->version = version;
}
void Nsvcap::set_context(const std::string & context) {
    p_impl->context = context;
}
void Nsvcap::set_arch(const std::string & arch) {
    p_impl->arch = arch;
}
void Nsvcap::set_profile(const std::string & profile) {
    p_impl->profile = profile;
}

void Nsvcap::set_name(std::string && name) {
    p_impl->name = std::move(name);
}
void Nsvcap::set_stream(std::string && stream) {
    p_impl->stream = std::move(stream);
}
void Nsvcap::set_version(std::string && version) {
    p_impl->version = std::move(version);
}
void Nsvcap::set_context(std::string && context) {
    p_impl->context = std::move(context);
}
void Nsvcap::set_arch(std::string && arch) {
    p_impl->arch = std::move(arch);
}
void Nsvcap::set_profile(std::string && profile) {
    p_impl->profile = std::move(profile);
}

}  // namespace libdnf5::module
