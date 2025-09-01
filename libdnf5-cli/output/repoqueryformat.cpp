// Copyright Contributors to the DNF5 project.
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


#include "fmt/args.h"
#include "utils/string.hpp"

#include "libdnf5-cli/output/repoquery.hpp"

#include <libdnf5/common/exception.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <set>
#include <variant>

namespace libdnf5::cli::output {

namespace {

using StrGetter = std::string (libdnf5::rpm::Package::*)() const;
using VecStrGetter = std::vector<std::string> (libdnf5::rpm::Package::*)() const;
using UnsignedLongLongGetter = unsigned long long (libdnf5::rpm::Package::*)() const;
using ReldepListGetter = libdnf5::rpm::ReldepList (libdnf5::rpm::Package::*)() const;
using TransactionItemReasonGetter = libdnf5::transaction::TransactionItemReason (libdnf5::rpm::Package::*)() const;
using StrGetterLambda = std::function<std::string(const libdnf5::rpm::Package &)>;

using Getter = std::variant<
    StrGetter,
    VecStrGetter,
    UnsignedLongLongGetter,
    ReldepListGetter,
    TransactionItemReasonGetter,
    StrGetterLambda>;

const std::unordered_map<std::string, Getter> NAME_TO_GETTER = {
    {"name", &libdnf5::rpm::Package::get_name},
    {"epoch", &libdnf5::rpm::Package::get_epoch},
    {"version", &libdnf5::rpm::Package::get_version},
    {"release", &libdnf5::rpm::Package::get_release},
    {"arch", &libdnf5::rpm::Package::get_arch},
    {"evr", &libdnf5::rpm::Package::get_evr},
    {"full_nevra", &libdnf5::rpm::Package::get_full_nevra},
    {"group", &libdnf5::rpm::Package::get_group},
    {"downloadsize", &libdnf5::rpm::Package::get_download_size},
    {"installsize", &libdnf5::rpm::Package::get_install_size},
    {"license", &libdnf5::rpm::Package::get_license},
    {"source_name", &libdnf5::rpm::Package::get_source_name},
    {"sourcerpm", &libdnf5::rpm::Package::get_sourcerpm},
    {"buildtime", &libdnf5::rpm::Package::get_build_time},
    {"packager", &libdnf5::rpm::Package::get_packager},
    {"vendor", &libdnf5::rpm::Package::get_vendor},
    {"url", &libdnf5::rpm::Package::get_url},
    {"summary", &libdnf5::rpm::Package::get_summary},
    {"description", &libdnf5::rpm::Package::get_description},
    {"provides", &libdnf5::rpm::Package::get_provides},
    {"requires", &libdnf5::rpm::Package::get_requires},
    {"requires_pre", &libdnf5::rpm::Package::get_requires_pre},
    {"conflicts", &libdnf5::rpm::Package::get_conflicts},
    {"obsoletes", &libdnf5::rpm::Package::get_obsoletes},
    {"prereq_ignoreinst", &libdnf5::rpm::Package::get_prereq_ignoreinst},
    {"regular_requires", &libdnf5::rpm::Package::get_regular_requires},
    {"recommends", &libdnf5::rpm::Package::get_recommends},
    {"suggests", &libdnf5::rpm::Package::get_suggests},
    {"enhances", &libdnf5::rpm::Package::get_enhances},
    {"supplements", &libdnf5::rpm::Package::get_supplements},
    {"depends", &libdnf5::rpm::Package::get_depends},
    {"from_repo", &libdnf5::rpm::Package::get_from_repo_id},
    {"installtime", &libdnf5::rpm::Package::get_install_time},
    {"repoid", &libdnf5::rpm::Package::get_repo_id},
    {"reponame", &libdnf5::rpm::Package::get_repo_name},
    {"reason", &libdnf5::rpm::Package::get_reason},
    {"debug_name", &libdnf5::rpm::Package::get_debuginfo_name},
    {"source_debug_name", &libdnf5::rpm::Package::get_debuginfo_name_of_source},
    {"files", &libdnf5::rpm::Package::get_files},
    {"location",
     [](const libdnf5::rpm::Package & pkg) -> std::string {
         auto locs = pkg.get_remote_locations();
         if (locs.empty()) {
             return {};
         }
         return locs[0];
     }},
};


// Argument format contains partially copied and converted queryformat, for example: "name: %-30{{name".
// We know the tag "name" is valid so this function check align spec "-30" and if it is valid
// it overwrites the format with fmt formatting: "name: {:<30}".
bool replace_tag_in_format(
    std::string & format,
    std::string::size_type & format_size,
    std::string::size_type tag_start,
    std::string::size_type tag_name_start) {
    std::string align_spec;
    // Check if there is valid align spec between '%' and '{' -> ..%-30{...
    if (tag_start + 1 != tag_name_start) {
        align_spec = format.substr(tag_start + 1, tag_name_start - tag_start - 1);
        auto iter = align_spec.begin();
        // first char can be either a digit or '-' sign
        if ((*iter != '-') && (std::isdigit(*iter) == 0)) {
            return false;
        }
        // verify the rest is only digits
        ++iter;
        while (iter != align_spec.end() && (std::isdigit(*iter) != 0)) {
            ++iter;
        }
        if (iter != align_spec.end()) {
            return false;
        }
    }

    // Since we use positional args for fmt::vprint we have to remove the tag name.
    // We do this by overwriting the previously copied tag.
    format_size = tag_start;
    format[format_size] = '{';
    format_size++;

    if (!align_spec.empty()) {
        format[format_size] = ':';
        format_size++;

        auto iter = align_spec.begin();
        if (*iter == '-') {
            format[format_size] = '<';
            iter++;  //skip the sign
        } else {
            format[format_size] = '>';
        }
        format_size++;
        while (iter != align_spec.end()) {
            format[format_size] = *iter;
            format_size++;
            ++iter;
        }
    }

    format[format_size] = '}';
    format_size++;
    return true;
}


std::pair<std::vector<Getter>, std::string> parse_queryformat(const std::string & queryformat) {
    std::vector<Getter> getters;
    std::string format;
    // format max possible len is 2 * queryformat.size() (if it contained just curly braces)
    format.resize(2 * queryformat.size());
    std::string::size_type format_size = 0;
    std::string::size_type tag_start = 0;
    std::string::size_type tag_name_start = 0;
    char previous_qf_char = 0;

    enum State { OUTSIDE, IN_TAG, IN_TAG_NAME };
    State state = OUTSIDE;

    // Prepare format string and getters.
    // We try to match rpm query formatting.
    // For example: "%-30{name}%{evr}" needs to be converted to "{:<30}{}" that is
    // two positional fmt arguments (first one with align formatting) and we need
    // get_name() and get_evr() getters.
    for (char qf_char : queryformat) {
        if (qf_char == '%') {  // start of tag
            state = IN_TAG;
            tag_start = format_size;
        } else if (qf_char == '{' && state == IN_TAG) {  // start of tag name
            state = IN_TAG_NAME;
            tag_name_start = format_size;
        } else if (qf_char == '}' && state == IN_TAG_NAME) {  // end of tag
            state = OUTSIDE;
            // To get the name we add/subtract 2 because each tag name (after brace expansion) starts with "{{".
            auto getter_name = format.substr(tag_name_start + 2, format_size - tag_name_start - 2);
            auto getter = NAME_TO_GETTER.find(libdnf5::utils::string::tolower(getter_name));
            if (getter != NAME_TO_GETTER.end()) {
                if (replace_tag_in_format(format, format_size, tag_start, tag_name_start)) {
                    getters.push_back(getter->second);
                    continue;  // continue to skip adding the current qf_char ('}')
                }
            }
        }

        // Copy each char of queryformat to format string with required changes.
        if (qf_char == '{' || qf_char == '}') {
            // double the brace -> escaping for fmt
            format[format_size] = qf_char;
            format_size++;
            format[format_size] = qf_char;
            format_size++;
        } else if (previous_qf_char == '\\' && qf_char == 'n') {
            // replace new lines, two characters in input: '\' 'n' -> '\n'
            format[format_size - 1] = '\n';  //replace the previous '\'
        } else {
            format[format_size] = qf_char;
            format_size++;
        }

        previous_qf_char = qf_char;
    }

    // Resize the format to resulting size to trim excess characters.
    format.resize(format_size);

    return {getters, format};
}

}  // namespace

bool requires_filelists(const std::string & queryformat) {
    auto [getters, _] = parse_queryformat(queryformat);
    for (auto getter : getters) {
        auto * getter_pointer = std::get_if<VecStrGetter>(&getter);
        if (getter_pointer && (*getter_pointer == &libdnf5::rpm::Package::get_files)) {
            return true;
        }
    }

    return false;
}


void print_pkg_set_with_format(
    std::FILE * target, const libdnf5::rpm::PackageSet & pkgs, const std::string & queryformat) {
    auto [getters, format] = parse_queryformat(queryformat);

    std::set<std::string> output;
    // Format and print each package.
    fmt::dynamic_format_arg_store<fmt::format_context> arg_store;
    arg_store.reserve(getters.size(), getters.size());
    for (auto package : pkgs) {
        arg_store.clear();
        for (const auto & getter : getters) {
            std::visit(
                [&arg_store, &package](const auto & getter_func) {
                    using T = std::decay_t<decltype(getter_func)>;
                    if constexpr (std::is_same_v<T, ReldepListGetter>) {
                        std::string joined;
                        for (const auto & reldep : (package.*getter_func)()) {
                            joined.append(std::move(reldep.to_string()));
                            joined.push_back('\n');
                        }
                        arg_store.push_back(joined);
                    } else if constexpr (std::is_same_v<T, VecStrGetter>) {
                        std::string joined;
                        for (const auto & str : (package.*getter_func)()) {
                            joined.append(std::move(str));
                            joined.push_back('\n');
                        }
                        arg_store.push_back(joined);
                    } else if constexpr (std::is_same_v<T, TransactionItemReasonGetter>) {
                        arg_store.push_back(std::move(transaction_item_reason_to_string((package.*getter_func)())));
                    } else if constexpr (std::is_same_v<T, StrGetterLambda>) {
                        arg_store.push_back((getter_func)(package));
                    } else {
                        arg_store.push_back((package.*getter_func)());
                    }
                },
                getter);
        }

        output.insert(fmt::vformat(format, arg_store));
    }

    for (const auto & line : output) {
        fmt::print(target, "{}", line);
    }
}


void print_pkg_attr_uniq_sorted(
    std::FILE * target, const libdnf5::rpm::PackageSet & pkgs, const std::string & getter_name) {
    auto getter = NAME_TO_GETTER.find(getter_name);
    if (getter == NAME_TO_GETTER.end()) {
        libdnf_throw_assertion("Package attribute getter: \"{}\" not available", getter_name);
    }
    std::set<std::string> output;
    for (auto package : pkgs) {
        std::visit(
            [&output, &package](const auto & getter_func) {
                using T = std::decay_t<decltype(getter_func)>;
                if constexpr (std::is_same_v<T, ReldepListGetter>) {
                    for (const auto & reldep : (package.*getter_func)()) {
                        output.insert(std::move(reldep.to_string()));
                    }
                } else if constexpr (std::is_same_v<T, UnsignedLongLongGetter>) {
                    output.insert(std::move(std::to_string((package.*getter_func)())));
                } else if constexpr (std::is_same_v<T, TransactionItemReasonGetter>) {
                    output.insert(std::move(transaction_item_reason_to_string((package.*getter_func)())));
                } else if constexpr (std::is_same_v<T, VecStrGetter>) {
                    for (const auto & str : (package.*getter_func)()) {
                        output.insert(std::move(str));
                    }
                } else if constexpr (std::is_same_v<T, StrGetterLambda>) {
                    output.insert(std::move((getter_func)(package)));
                } else {
                    output.insert(std::move((package.*getter_func)()));
                }
            },
            getter->second);
    }

    for (const auto & line : output) {
        fmt::print(target, "{}\n", line);
    }
}


void print_available_pkg_attrs(std::FILE * target) {
    std::set<std::string> output;
    for (const auto & pair : NAME_TO_GETTER) {
        output.insert(pair.first);
    }
    for (const auto & line : output) {
        fmt::print(target, "{}\n", line);
    }
}


libdnf5::rpm::ReldepList get_reldeplist_for_attr(
    const libdnf5::rpm::PackageSet & pkgs, const std::string & getter_name) {
    auto getter = NAME_TO_GETTER.find(getter_name);
    if (getter == NAME_TO_GETTER.end()) {
        libdnf_throw_assertion("Package attribute getter: \"{}\" not available", getter_name);
    }
    rpm::ReldepList output(pkgs.get_base());

    for (auto package : pkgs) {
        std::visit(
            [&output, &package, &getter_name](const auto & getter_func) {
                using T = std::decay_t<decltype(getter_func)>;
                if constexpr (std::is_same_v<T, ReldepListGetter>) {
                    auto rlds = (package.*getter_func)();
                    output.append(rlds);
                } else {
                    libdnf_throw_assertion("Cannot get pkg attribute: \"{}\" as ReldepList", getter_name);
                }
            },
            getter->second);
    }

    return output;
}

}  // namespace libdnf5::cli::output
