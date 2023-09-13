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

#include "libdnf5/conf/vars.hpp"

#include "rpm/arch_private.hpp"
#include "rpm/rpm_log_guard.hpp"
#include "utils/fs/file.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <dirent.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmmacro.h>
#include <rpm/rpmts.h>
#include <stdlib.h>
#include <sys/types.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>

#define ASCII_LOWERCASE "abcdefghijklmnopqrstuvwxyz"
#define ASCII_UPPERCASE "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ASCII_LETTERS   ASCII_LOWERCASE ASCII_UPPERCASE
#define DIGITS          "0123456789"

extern char ** environ;

namespace libdnf5 {

static const std::unordered_set<std::string> READ_ONLY_VARIABLES = {"releasever_major", "releasever_minor"};

// ==================================================================
// The following helper functions should be moved e.g. into a library

static void init_lib_rpm(const char * arch) {
    static bool lib_rpm_initiated{false};
    if (!lib_rpm_initiated) {
        if (rpmReadConfigFiles(nullptr, arch) != 0) {
            throw RuntimeError(M_("failed to read rpm config files"));
        }
        lib_rpm_initiated = true;
    }
}

static constexpr const char * DISTROVERPKGS[] = {
    "system-release(releasever)",
    "system-release",
    "distribution-release(releasever)",
    "distribution-release",
    "redhat-release",
    "suse-release"};

static std::string detect_arch() {
    std::string value{};
    char * tmp = rpmExpand("%{_host_cpu}", NULL);
    if (tmp != nullptr) {
        value = tmp;
        free(tmp);
    }
    return value;
}


// ==================================================================


std::unique_ptr<std::string> Vars::detect_release(const BaseWeakPtr & base, const std::string & install_root_path) {
    std::unique_ptr<std::string> release_ver;

    libdnf5::rpm::RpmLogGuard rpm_log_guard(base);

    auto ts = rpmtsCreate();
    rpmtsSetRootDir(ts, install_root_path.c_str());
    for (auto distroverpkg : DISTROVERPKGS) {
        auto mi = rpmtsInitIterator(ts, RPMTAG_PROVIDENAME, distroverpkg, 0);
        if (auto hdr = rpmdbNextIterator(mi)) {
            const char * version = headerGetString(hdr, RPMTAG_VERSION);
            rpmds ds = rpmdsNew(hdr, RPMTAG_PROVIDENAME, 0);
            while (rpmdsNext(ds) >= 0) {
                if (std::strcmp(rpmdsN(ds), distroverpkg) == 0 && rpmdsFlags(ds) == RPMSENSE_EQUAL) {
                    const char * evr = rpmdsEVR(ds);
                    if (evr && evr[0] != '\0') {
                        version = evr;
                        break;
                    }
                }
            }
            if (version) {
                // Is the result of rpmdsEVR(ds) valid after rpmdsFree(ds)? Make a copy to be sure.
                release_ver = std::make_unique<std::string>(version);
            }
            rpmdsFree(ds);
        }
        rpmdbFreeIterator(mi);
        if (release_ver) {
            break;
        }
    }
    rpmtsFree(ts);
    return release_ver;
}


Vars::Vars(Base & base) : Vars(base.get_weak_ptr()) {}

const unsigned int MAXIMUM_EXPRESSION_DEPTH = 32;

// Expand variables in a subexpression
//
// @param text String with variable expressions
// @param depth The recursive depth
// @return Pair of the resulting string and the number of characters scanned in `text`
std::pair<std::string, size_t> Vars::substitute_expression(std::string_view text, unsigned int depth) const {
    if (depth > MAXIMUM_EXPRESSION_DEPTH) {
        return std::make_pair(std::string(text), text.length());
    }
    std::string res{text};

    // The total number of characters read in the replacee
    size_t total_scanned = 0;

    size_t pos = 0;
    while (pos < res.length()) {
        if (res[pos] == '}' && depth > 0) {
            return std::make_pair(res.substr(0, pos), total_scanned);
        }

        if (res[pos] == '\\') {
            // Escape the next character (if there is one)
            if (pos + 1 >= res.length()) {
                break;
            }
            res.erase(pos, 1);
            total_scanned += 2;
            pos += 1;
            continue;
        }
        if (res[pos] == '$') {
            // variable expression starts after the $ and includes the braces
            //     ${variable:-word}
            //      ^-- pos_variable_expression
            size_t pos_variable_expression = pos + 1;
            if (pos_variable_expression >= res.length()) {
                break;
            }

            // Does the variable expression use braces? If so, the variable name
            // starts one character after the start of the variable_expression
            bool has_braces;
            size_t pos_variable;
            if (res[pos_variable_expression] == '{') {
                has_braces = true;
                pos_variable = pos_variable_expression + 1;
                if (pos_variable >= res.length()) {
                    break;
                }
            } else {
                has_braces = false;
                pos_variable = pos_variable_expression;
            }

            // Find the end of the variable name
            auto it = std::find_if_not(res.begin() + static_cast<long>(pos_variable), res.end(), [](char c) {
                return std::isalnum(c) != 0 || c == '_';
            });
            auto pos_after_variable = static_cast<size_t>(std::distance(res.begin(), it));

            // Find the substituting string and the end of the variable expression
            auto variable_mapping = variables.find(res.substr(pos_variable, pos_after_variable - pos_variable));
            const std::string * subst_str = nullptr;

            size_t pos_after_variable_expression;

            if (has_braces) {
                if (pos_after_variable >= res.length()) {
                    break;
                }
                if (res[pos_after_variable] == ':') {
                    if (pos_after_variable + 1 >= res.length()) {
                        break;
                    }
                    char expansion_mode = res[pos_after_variable + 1];
                    size_t pos_word = pos_after_variable + 2;
                    if (pos_word >= res.length()) {
                        break;
                    }

                    // Expand the default/alternate expression
                    auto word_str = std::string_view(res).substr(pos_word);
                    auto [expanded_word, scanned] = substitute_expression(word_str, depth + 1);
                    auto pos_after_word = pos_word + scanned;
                    if (pos_after_word >= res.length()) {
                        break;
                    }
                    if (res[pos_after_word] != '}') {
                        // The variable expression doesn't end in a '}',
                        // continue after the word and don't expand it
                        total_scanned += pos_after_word - pos;
                        pos = pos_after_word;
                        continue;
                    }

                    if (expansion_mode == '-') {
                        // ${variable:-word} (default value)
                        // If variable is unset or empty, the expansion of word is
                        // substituted. Otherwise, the value of variable is
                        // substituted.
                        if (variable_mapping == variables.end() || variable_mapping->second.value.empty()) {
                            subst_str = &expanded_word;
                        } else {
                            subst_str = &variable_mapping->second.value;
                        }
                    } else if (expansion_mode == '+') {
                        // ${variable:+word} (alternate value)
                        // If variable is unset or empty nothing is substituted.
                        // Otherwise, the expansion of word is substituted.
                        if (variable_mapping == variables.end() || variable_mapping->second.value.empty()) {
                            const std::string empty{};
                            subst_str = &empty;
                        } else {
                            subst_str = &expanded_word;
                        }
                    } else {
                        // Unknown expansion mode, continue after the ':'
                        pos = pos_after_variable + 1;
                        continue;
                    }
                    pos_after_variable_expression = pos_after_word + 1;
                } else if (res[pos_after_variable] == '}') {
                    // ${variable}
                    if (variable_mapping != variables.end()) {
                        subst_str = &variable_mapping->second.value;
                    }
                    // Move past the closing '}'
                    pos_after_variable_expression = pos_after_variable + 1;
                } else {
                    // Variable expression doesn't end in a '}', continue after the variable
                    pos = pos_after_variable;
                    continue;
                }
            } else {
                // No braces, we have a $variable
                if (variable_mapping != variables.end()) {
                    subst_str = &variable_mapping->second.value;
                }
                pos_after_variable_expression = pos_after_variable;
            }

            // If there is no substitution to make, move past the variable expression and continue.
            if (subst_str == nullptr) {
                total_scanned += pos_after_variable_expression - pos;
                pos = pos_after_variable_expression;
                continue;
            }

            res.replace(pos, pos_after_variable_expression - pos, *subst_str);
            total_scanned += pos_after_variable_expression - pos;
            pos += subst_str->length();
        } else {
            total_scanned += 1;
            pos += 1;
        }
    }

    // We have reached the end of the text
    if (depth > 0) {
        // If we are in a subexpression and we didn't find a closing '}', make no substitutions.
        return std::make_pair(std::string{text}, text.length());
    }

    return std::make_pair(res, text.length());
}

std::string Vars::substitute(const std::string & text) const {
    return substitute_expression(text, 0).first;
}

std::tuple<std::string, std::string> Vars::split_releasever(const std::string & releasever) {
    // Uses the same logic as splitReleaseverTo in libzypp
    std::string releasever_major;
    std::string releasever_minor;
    const auto pos = releasever.find('.');
    if (pos == std::string::npos) {
        releasever_major = releasever;
    } else {
        releasever_major = releasever.substr(0, pos);
        releasever_minor = releasever.substr(pos + 1);
    }
    return std::make_tuple(releasever_major, releasever_minor);
}

bool Vars::is_read_only(const std::string & name) const {
    return READ_ONLY_VARIABLES.contains(name);
}

void Vars::set(const std::string & name, const std::string & value, Priority prio) {
    if (is_read_only(name)) {
        throw ReadOnlyVariableError(M_("Variable \"{}\" is read-only"), name);
    }

    // set_unsafe sets the variable without checking whether it's read-only
    std::function<void(const std::string, const std::string, Priority)> set_unsafe =
        [&](const std::string & name, const std::string & value, Priority prio) {
            auto it = variables.find(name);

            // Do nothing if the var is already set with a higher priority
            if (it != variables.end() && prio < it->second.priority) {
                return;
            }

            // Whenever releasever is set, split it into major and minor parts
            if (name == "releasever") {
                const auto [releasever_major, releasever_minor] = split_releasever(value);
                if (!releasever_major.empty()) {
                    set_unsafe("releasever_major", releasever_major, prio);
                }
                if (!releasever_minor.empty()) {
                    set_unsafe("releasever_minor", releasever_minor, prio);
                }
            }

            if (it == variables.end()) {
                variables.insert({name, {value, prio}});
            } else {
                it->second.value = value;
                it->second.priority = prio;
            }
        };
    set_unsafe(name, value, prio);
}

void Vars::set_lazy(
    const std::string & name,
    const std::function<const std::unique_ptr<const std::string>()> & get_value,
    const Priority prio) {
    auto it = variables.find(name);
    if (it == variables.end() || prio > it->second.priority) {
        const auto maybe_value = get_value();
        if (maybe_value != nullptr) {
            set(name, *maybe_value, prio);
        }
    }
}

void Vars::load(const std::string & installroot, const std::vector<std::string> & directories) {
    load_from_env();

    const std::filesystem::path installroot_path{installroot};
    for (const auto & dir : directories) {
        load_from_dir(installroot_path / std::filesystem::path(dir).relative_path());
    }

    detect_vars(installroot);
}

void Vars::detect_vars(const std::string & installroot) {
    const char * arch = nullptr;
    if (contains("arch")) {
        arch = get_value("arch").c_str();
    }
    init_lib_rpm(arch);

    set_lazy(
        "arch", []() -> auto { return std::make_unique<std::string>(detect_arch()); }, Priority::AUTO);

    set_lazy(
        "basearch",
        [this]() -> auto {
            auto base_arch = libdnf5::rpm::get_base_arch(variables["arch"].value.c_str());
            return base_arch ? std::make_unique<std::string>(base_arch) : nullptr;
        },
        Priority::AUTO);

    set_lazy(
        "releasever", [this, &installroot]() -> auto { return detect_release(base, installroot); }, Priority::AUTO);
}

static void dir_close(DIR * d) {
    closedir(d);
}

void Vars::load_from_dir(const std::string & directory) {
    auto & logger = *base->get_logger();
    if (DIR * dir = opendir(directory.c_str())) {
        std::unique_ptr<DIR, decltype(&dir_close)> dir_guard(dir, &dir_close);
        while (auto ent = readdir(dir)) {
            auto dname = ent->d_name;
            if (dname[0] == '.' && (dname[1] == '\0' || (dname[1] == '.' && dname[2] == '\0')))
                continue;

            auto full_path = directory;
            if (full_path.back() != '/') {
                full_path += "/";
            }
            full_path += dname;

            std::string line;
            try {
                utils::fs::File file(full_path, "r");
                file.read_line(line);
            } catch (const std::filesystem::filesystem_error & e) {
                logger.warning("Cannot load variable from file \"{}\": {}", full_path.c_str(), e.what());
                continue;
            }
            set(dname, line, Priority::VARSDIR);
        }
    }
}

void Vars::load_from_env() {
    if (!environ) {
        return;
    }

    for (const char * const * var_ptr = environ; *var_ptr; ++var_ptr) {
        auto var = *var_ptr;
        if (auto eql_ptr = strchr(var, '=')) {
            auto eql_idx = eql_ptr - var;
            if (eql_idx == 4 && strncmp("DNF", var, 3) == 0 && isdigit(var[3]) != 0) {
                // DNF[0-9]
                set(std::string(var, static_cast<size_t>(eql_idx)), eql_ptr + 1, Priority::ENVIRONMENT);
            } else if (
                // DNF_VAR_[A-Za-z0-9_]+ , DNF_VAR_ prefix is cut off
                eql_idx > 8 && strncmp("DNF_VAR_", var, 8) == 0 &&
                static_cast<int>(strspn(var + 8, ASCII_LETTERS DIGITS "_")) == eql_idx - 8) {
                set(std::string(var + 8, static_cast<size_t>(eql_idx - 8)), eql_ptr + 1, Priority::ENVIRONMENT);
            }
        }
    }
}

}  // namespace libdnf5
