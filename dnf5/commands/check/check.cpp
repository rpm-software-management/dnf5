/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "check.hpp"

#include <fmt/format.h>
#include <libdnf5/rpm/nevra.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmps.h>
#include <rpm/rpmtag.h>
#include <rpm/rpmtd.h>
#include <rpm/rpmte.h>
#include <rpm/rpmts.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include <ranges>
#include <filesystem>
#include <cstddef>
namespace dnf5 {

using namespace libdnf5;

namespace {

enum class ProblemType { MISSING_REQUIRE, CONFLICT, OBSOLETED, DUPLICATE, PATH_CONFLICT };

// Package is identified by "nevra". But there may be a situation where there are several packages with
// the same "nevra" in the system (broken installation?).
// That's why we also keep a unique package index from the rpm database.
struct PkgId {
    std::string nevra;
    std::invoke_result_t<decltype(&rpmteDBOffset), rpmte> rpmdb_idx;

    bool operator==(const PkgId & p) const noexcept { return rpmdb_idx == p.rpmdb_idx; }

    std::strong_ordering operator<=>(const PkgId & p) const noexcept {
        auto cmp = nevra <=> p.nevra;
        return cmp != 0 ? cmp : rpmdb_idx <=> p.rpmdb_idx;
    }
};

// Describes one problem
struct Problem {
    ProblemType type;             // type of problem
    std::string nevra;            // "nevra" causing this problem
    std::string file_or_provide;  // "file" or "provide" causing this problem

    std::strong_ordering operator<=>(const Problem & p) const noexcept {
        if (auto cmp = type <=> p.type; cmp != 0) {
            return cmp;
        }
        if (auto cmp = file_or_provide <=> p.file_or_provide; cmp != 0) {
            return cmp;
        }
        return nevra <=> p.nevra;
    }
};

// what the program will check
bool dependencies{false};
bool duplicates{false};
bool obsoleted{false};

// Contains "provides" to recognize the installonly package.
// It is initialized with values from the configuration.
std::vector<std::string> installonly_pkgs_provides;

// Contains a set of problems for each problematic package.
// The std::map Key is the "nevra" of the problematic package.
std::map<PkgId, std::set<Problem>> problems;

// Contains a list of installed packages for each "name.arch".
// For "installonly" packages, only one package is present.
// Usually contains 1 package for "name.arch". Multiple packages means "duplicates".
std::map<std::string, std::vector<PkgId>> installed_na_packages;


cli::ArgumentParser::NamedArg * create_option(
    cli::ArgumentParser & parser, const std::string & name, const std::string & descr, bool & opt_to_modify) {
    auto option = parser.add_new_named_arg(name);
    option->set_long_name(name);
    option->set_description(descr);
    option->set_parse_hook_func([&opt_to_modify](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        opt_to_modify = true;
        return true;
    });
    return option;
}


inline std::map<std::string, std::vector<std::string>> has_duplicate_in_PATH() {
    std::map<std::string, std::vector<std::string>> PATH_dir;
    std::map<std::string, std::vector<std::string>> Duplicates;
    const std::string PATH = getenv("PATH");

    // parse PATH variable
    for (const auto path : PATH | std::views::split(':')) {
        PATH_dir.insert({path.data(), {}});
    }

    // populate file list
    for (const auto& [dir, _] : PATH_dir) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            std::string path = entry.path().string();
            std::size_t found = path.find_last_of("/\\");
            PATH_dir[dir].push_back(path.substr(found + 1));
        }
    }

    //generate vector with all files
    std::vector<std::string> Total_vec;
    std::set<std::string> Duplicate_commands;
    for (const auto& [key, vec] : PATH_dir) {
        //prealoocate memory
        Total_vec.reserve(Total_vec.size() + vec.size());
        Total_vec.insert( Total_vec.end(), vec.begin(), vec.end() );
    }
    std::sort(Total_vec.begin(), Total_vec.end());
    //collapse vector into set
    std::set<std::string> Total_set(std::make_move_iterator(Total_vec.begin()),
                                    std::make_move_iterator(Total_vec.end()));
    // generate duplicated programs
    set_difference(Total_vec.begin(), Total_vec.end(),
                   Total_set.begin(), Total_set.end(),
                   std::inserter(Duplicate_commands, Duplicate_commands.begin()));
    //quick return path
    if (Duplicate_commands.empty()) { return Duplicates;}
    else{
        //populate map
        for(const auto& item : Duplicate_commands) {
            Duplicates.insert({item, {}});
        }
        //fill out map
        for(const auto& [key, _] : Duplicates) {
            //compare value to find ofending path
            for(const auto& [innerkey, files] : PATH_dir) {
                if(std::find(files.begin(), files.end(), key) != files.end()) {
                    Duplicates[key].insert(Duplicates[key].end(), innerkey);
                }
            }
        }
    }

    return Duplicates;
}

inline bool is_package_installonly(Header h) noexcept {
    rpmtd td = rpmtdNew();
    headerGet(h, RPMTAG_PROVIDES, td, 0);
    while (const auto * const provide = rpmtdNextString(td)) {
        for (const auto & installonly_pkg_provide : installonly_pkgs_provides) {
            if (installonly_pkg_provide == provide) {
                rpmtdFree(td);
                return true;
            }
        }
    }
    rpmtdFree(td);

    return false;
}


// Ensures that NEVRA always contains an EPOCH.
// Returns the input `nevra`. EPOCH is inserted if it was not on the input.
// librpm functions such as "rpmProblemGetPkgNEVR()" and "rpmProblemGetAltNEVR()" return epoch 0 only sometimes.
// We want the epoch always.
inline std::string ensure_full_nevra(const char * nevra) {
    const auto nevras = rpm::Nevra::parse(nevra, {rpm::Nevra::Form::NEVRA});
    return rpm::to_full_nevra_string(nevras.front());
}


// Finds problems for the installed package defined by Header h.
// Principle: Adds the installed package to an empty transaction and checks for errors.
void find_problems(rpmts ts, Header h) {
    int rc = 0;

    rpmtsEmpty(ts);
    (void)rpmtsAddInstallElement(ts, h, NULL, 0, NULL);

    (void)rpmtsCheck(ts);
    auto te = rpmtsElement(ts, 0);
    auto ps = rpmteProblems(te);
    rc = rpmpsNumProblems(ps);

    if (rc > 0) {
        auto rpmdb_idx = rpmteDBOffset(te);
        rpmpsi psi = rpmpsInitIterator(ps);
        while (auto p = rpmpsiNext(psi)) {
            const auto * const problem_cstr = rpmProblemGetStr(p);
            libdnf_assert(problem_cstr != NULL, "command \"check\": rpmProblemGetStr() returns NULL");
            const auto * const cnevra = rpmProblemGetPkgNEVR(p);
            libdnf_assert(cnevra != NULL, "command \"check\": rpmProblemGetPkgNEVR() returns NULL");
            const auto * const alt_cnevra = rpmProblemGetAltNEVR(p);
            libdnf_assert(alt_cnevra != NULL, "command \"check\": rpmProblemGetAltNEVR() returns NULL");

            switch (rpmProblemGetType(p)) {
                case RPMPROB_REQUIRES:
                    if (dependencies) {
                        problems[{ensure_full_nevra(alt_cnevra), rpmdb_idx}].insert(Problem{
                            .type = ProblemType::MISSING_REQUIRE, .nevra = "", .file_or_provide = problem_cstr});
                    }
                    break;
                case RPMPROB_CONFLICT:
                    if (dependencies) {
                        auto nevra = ensure_full_nevra(cnevra);
                        auto alt_nevra = ensure_full_nevra(alt_cnevra);
                        if (nevra == alt_nevra) {
                            // skip self conflicts
                            break;
                        }
                        problems[{nevra, rpmdb_idx}].insert(Problem{
                            .type = ProblemType::CONFLICT, .nevra = alt_nevra, .file_or_provide = problem_cstr});
                    }
                    break;
                case RPMPROB_OBSOLETES:
                    if (obsoleted) {
                        auto nevra = ensure_full_nevra(cnevra);
                        auto alt_nevra = ensure_full_nevra(alt_cnevra);
                        if (nevra == alt_nevra) {
                            // skip self obsolete
                            break;
                        }
                        problems[{nevra, rpmdb_idx}].insert(Problem{
                            .type = ProblemType::OBSOLETED, .nevra = alt_nevra, .file_or_provide = problem_cstr});
                    }
                    break;
                default:;
            }
        }
        rpmpsFreeIterator(psi);
    }
    rpmpsFree(ps);
    rpmtsEmpty(ts);
}


// Groups packages with the same "name" and "architecture".
// For "installonly" packages, only one "nevra" is inserted.
void group_pkgs_same_name_arch(Header h) {
    const auto * const arch = headerGetString(h, RPMTAG_ARCH);
    if (!arch) {
        // skip any non-packages (such as gpg-pubkey) in the database
        return;
    }

    const auto * const name = headerGetString(h, RPMTAG_NAME);

    auto & nevras = installed_na_packages[fmt::format("{}.{}", name, arch)];
    if (nevras.empty() || !is_package_installonly(h)) {
        // "duplicate" packages are only added if they are not "installonly" packages
        // to avoid being reported as duplicates.

        // "headerGetAsString(h, RPMTAG_NEVRA)" returns epoch 0 only sometimes.
        // We want the epoch always. We build NEVRA ourselves.
        const auto epoch = headerGetNumber(h, RPMTAG_EPOCH);
        const auto * const version = headerGetString(h, RPMTAG_VERSION);
        const auto * const release = headerGetString(h, RPMTAG_RELEASE);
        auto rpmdb_idx = static_cast<decltype(PkgId::rpmdb_idx)>(headerGetInstance(h));
        nevras.push_back({fmt::format("{}-{}:{}-{}.{}", name, epoch, version, release, arch), rpmdb_idx});
    }
}

}  // namespace


void CheckCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}


void CheckCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Check for problems in the packagedb");
    cmd.set_long_description(
        "Checks the local packagedb and produces information on any problems it finds.\n"
        "The set of checks performed can be specified with options.");

    cmd.register_named_arg(
        create_option(parser, "dependencies", "Show missing dependencies and conflicts", dependencies));
    cmd.register_named_arg(create_option(parser, "duplicates", "Show duplicated packages", duplicates));
    cmd.register_named_arg(create_option(parser, "obsoleted", "Show obsoleted packages", obsoleted));
}


void CheckCommand::configure() {
    installonly_pkgs_provides = get_context().get_base().get_config().get_installonlypkgs_option().get_value();
    if (!(dependencies || duplicates || obsoleted)) {
        dependencies = duplicates = obsoleted = true;
    }
}


void CheckCommand::run() {
    auto & ctx = get_context();

    auto ts = rpmtsCreate();
    auto & config = ctx.get_base().get_config();
    rpmtsSetRootDir(ts, config.get_installroot_option().get_value().c_str());

    rpmdbMatchIterator mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);
    std::unique_ptr<std::remove_pointer_t<rpmdbMatchIterator>, decltype(&rpmdbFreeIterator)> mi_owner(
        mi, &rpmdbFreeIterator);
    while (Header h = rpmdbNextIterator(mi)) {
        if (dependencies || obsoleted) {
            find_problems(ts, h);
        }
        if (duplicates) {
            group_pkgs_same_name_arch(h);
        }
    }

    if (duplicates) {
        for (const auto & [na, packages] : installed_na_packages) {
            if (packages.size() > 1) {
                for (const auto & package : packages) {
                    for (const auto & duplicate_package : packages) {
                        if (package == duplicate_package) {
                            // skip itself
                            continue;
                        }
                        problems[package].insert(Problem{
                            .type = ProblemType::DUPLICATE, .nevra = duplicate_package.nevra, .file_or_provide = ""});
                    }
                }
            }
        }
    }

    rpmtsFree(ts);

    // Print problems
    if (!problems.empty()) {
        std::size_t problem_count{0};
        for (const auto & [package_id, problems_list] : problems) {
            std::cout << package_id.nevra << std::endl;
            for (const auto & problem : problems_list) {
                ++problem_count;
                switch (problem.type) {
                    case ProblemType::MISSING_REQUIRE:
                        std::cout << fmt::format(" missing require \"{}\"", problem.file_or_provide) << std::endl;
                        break;
                    case ProblemType::CONFLICT:
                        std::cout << fmt::format(
                                         " installed conflict \"{}\" from \"{}\"",
                                         problem.file_or_provide,
                                         problem.nevra)
                                  << std::endl;
                        break;
                    case ProblemType::OBSOLETED:
                        std::cout << fmt::format(
                                         " obsoleted by \"{}\" from \"{}\"", problem.file_or_provide, problem.nevra)
                                  << std::endl;
                        break;
                    case ProblemType::DUPLICATE:
                        std::cout << fmt::format(" duplicate with \"{}\"", problem.nevra) << std::endl;
                        break;
                }
            }
        }

        throw Error(M_("Check discovered {} problem(s) in {} package(s)"), problem_count, problems.size());
    }
}

}  // namespace dnf5
