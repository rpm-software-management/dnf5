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

#include "libdnf/module/module_sack.hpp"

#include "module/module_goal_private.hpp"
#include "module/module_metadata.hpp"
#include "module/module_sack_impl.hpp"
#include "solv/solv_map.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/fs/file.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/base/base_weak.hpp"
#include "libdnf/module/module_errors.hpp"
#include "libdnf/module/module_item.hpp"
#include "libdnf/module/module_query.hpp"
#include "libdnf/module/module_sack_weak.hpp"

#include <modulemd-2.0/modulemd.h>

extern "C" {
#include <solv/pool.h>
#include <solv/repo.h>
}

#include "../rpm/package_sack_impl.hpp"

#include "libdnf/repo/repo_weak.hpp"
#include "libdnf/rpm/package_query.hpp"

#include <memory>
#include <string>

namespace libdnf::module {


static const std::string EMPTY_RESULT;


ModuleSack::ModuleSack(const BaseWeakPtr & base) : p_impl(new Impl(*this, base)) {}
ModuleSack::~ModuleSack() {}


const std::vector<std::unique_ptr<ModuleItem>> & ModuleSack::get_modules() {
    return p_impl->get_modules();
}


std::vector<ModuleItem *> ModuleSack::get_active_modules() {
    std::vector<ModuleItem *> modules;
    if (p_impl->get_modules().empty()) {
        return modules;
    }
    if (!active_modules_resolved) {
        resolve_active_module_items();
    }
    for (auto id_module_pair : p_impl->active_modules) {
        modules.push_back(id_module_pair.second);
    }
    return modules;
}


void ModuleSack::add(const std::string & file_content, const std::string & repo_id) {
    ModuleMetadata md(get_base());
    try {
        md.add_metadata_from_string(file_content, 0);
        // The second call to add_metadata_from_string is to load all metadata in to `p_impl->module_metadata` and use
        // them later to get all defaults.
        p_impl->module_metadata.add_metadata_from_string(file_content, 0);
    } catch (const ModuleResolveError & e) {
        throw ModuleResolveError(
            M_("Failed to load module metadata for repository \"{}\": {}"), repo_id, std::string(e.what()));
    }

    Repo * repo;
    auto repo_pair = p_impl->repositories.find(repo_id);
    if (repo_pair == p_impl->repositories.end()) {
        repo = repo_create(p_impl->pool, repo_id.c_str());
        p_impl->repositories[repo_id] = int(repo->repoid);
    } else {
        repo = pool_id2repo(p_impl->pool, Id(repo_pair->second));
    }
    auto items = md.get_all_module_items(get_weak_ptr(), repo_id);
    // Store module items with static context
    for (auto const & module_item_ptr : items.first) {
        std::unique_ptr<ModuleItem> module_item(module_item_ptr);
        module_item->create_solvable_and_dependencies();
        p_impl->modules.push_back(std::move(module_item));
    }
    // Store module items without static context
    for (auto const & module_item_ptr : items.second) {
        std::unique_ptr<ModuleItem> module_item(module_item_ptr);
        p_impl->modules_without_static_context.push_back(std::move(module_item));
    }
}


void ModuleSack::Impl::add_modules_without_static_context() {
    if (modules_without_static_context.empty()) {
        return;
    }

    // Create a map based on modules with static context. For each "name:stream", map requires_strings to ModuleItems.
    std::map<std::string, std::map<std::string, std::vector<ModuleItem *>>> static_context_map;
    for (auto const & module_item : modules) {
        auto requires_string = module_item->get_module_dependencies_string();
        static_context_map[module_item->get_name_stream()][requires_string].push_back(module_item.get());
    }

    // For each module with dynamic context, check whether its requires_string matches requires_string of any
    // static-context module with the same "name:stream" (i.e. if it's in the static_context_map). If so, assign it
    // the same static context.
    for (auto & module_item : modules_without_static_context) {
        auto requires_string = module_item->get_module_dependencies_string();

        auto stream_iterator = static_context_map.find(module_item->get_name_stream());
        if (stream_iterator != static_context_map.end()) {
            auto context_iterator = stream_iterator->second.find(requires_string);
            if (context_iterator != stream_iterator->second.end()) {
                module_item->computed_static_context = context_iterator->second[0]->get_context();
                module_item->create_solvable_and_dependencies();
                modules.push_back(std::move(module_item));
                continue;
            }
        }
        // If the requires_string didn't match anything in the static_context_map, set the new static context to
        // the requires_string (or "NoRequires" if empty). This means all dynamic-context modules with the same
        // "name:stream" and the same dependencies will have the same static context.
        if (requires_string.empty()) {
            requires_string.append("NoRequires");
        }
        module_item->computed_static_context = requires_string;
        module_item->create_solvable_and_dependencies();
        modules.push_back(std::move(module_item));
    }
    modules_without_static_context.clear();
}


ModuleSackWeakPtr ModuleSack::get_weak_ptr() {
    return ModuleSackWeakPtr(this, &data_guard);
}


BaseWeakPtr ModuleSack::get_base() const {
    return p_impl->base;
}


const std::string & ModuleSack::get_default_stream(const std::string & name) const {
    p_impl->module_defaults = p_impl->module_metadata.get_default_streams();
    auto it = p_impl->module_defaults.find(name);
    if (it == p_impl->module_defaults.end()) {
        return EMPTY_RESULT;
    }
    return it->second;
}


std::vector<std::string> ModuleSack::get_default_profiles(std::string module_name, std::string module_stream) {
    return p_impl->module_metadata.get_default_profiles(module_name, module_stream);
}


std::tuple<
    std::vector<std::string>,
    std::vector<std::string>,
    std::vector<std::string>,
    std::vector<std::string>,
    rpm::ReldepList>
ModuleSack::Impl::collect_data_for_modular_filtering() {
    // TODO(jmracek) Add support of demodularized RPMs
    // auto demodularizedNames = getDemodularizedRpms(modulePackageContainer, allPackages);

    std::vector<std::string> include_NEVRAs;
    std::vector<std::string> exclude_NEVRAs;
    std::vector<std::string> names;
    std::vector<std::string> src_names;
    libdnf::rpm::ReldepList reldep_name_list(base);
    for (const auto & module : get_modules()) {
        auto artifacts = module->get_artifacts();
        if (module->is_active()) {
            // TODO(jmracek) Add support of demodularized RPMs
            // std::string package_ID{module->get_name_stream()};
            // package_ID.append(".");
            // package_ID.append(module->get_arch());
            //auto it = demodularizedNames.find(package_ID);
            //if (it == demodularizedNames.end()) {
            if (true) {
                for (const auto & rpm : artifacts) {
                    auto nevras = rpm::Nevra::parse(rpm, {rpm::Nevra::Form::NEVRA});
                    if (nevras.empty()) {
                        // TODO(jmracek) Unparsable NEVRA - What to do?
                        continue;
                    }
                    auto & nevra = *nevras.begin();
                    auto arch = nevra.get_arch();
                    if (arch == "src" || arch == "nosrc") {
                        src_names.push_back(nevra.get_name());
                    } else {
                        names.push_back(nevra.get_name());
                        reldep_name_list.add_reldep(nevra.get_name());
                    }
                }
            }
            // } else {
            //     for (const auto &rpm : artifacts) {
            //         auto nevras = rpm::Nevra::parse(rpm, {rpm::Nevra::Form::NEVRA});
            //         if (nevras.empty()) {
            //             // TODO(jmracek) Unparsable NEVRA - What to do?
            //             continue;rpm/package_query.hpp
            //         }
            //         auto & nevra = *nevras.begin();
            //         bool found = false;
            //         for ( auto & demodularized : it->second) {
            //             if (nevra.get_name() == demodularized) {
            //                 found = true;
            //                 break;
            //             }
            //         }
            //         if (found) {
            //             continue;
            //         }
            //         auto arch = nevra.get_arch();
            //         // source packages do not provide anything and must not cause excluding binary packages
            //         if (arch == "src" || arch == "nosrc") {
            //             src_names.push_back(nevra.get_name());
            //         } else {
            //             names.push_back(nevra.get_name());
            //             reldep_name_list.add_reldep(nevra.get_name());
            //         }
            //     }rpm/package_query.hpp
            // }
            copy(std::begin(artifacts), std::end(artifacts), std::back_inserter(include_NEVRAs));
        } else {
            copy(std::begin(artifacts), std::end(artifacts), std::back_inserter(exclude_NEVRAs));
        }
    }

    return std::make_tuple(
        std::move(include_NEVRAs),
        std::move(exclude_NEVRAs),
        std::move(names),
        std::move(src_names),
        std::move(reldep_name_list));
}

void ModuleSack::Impl::module_filtering() {
    if (get_modules().empty()) {
        return;
    }

    // Try to automatically detect platform id
    if (!platform_detected) {
        auto platform_id = detect_platform_name_and_stream();
        if (platform_id) {
            ModuleItem::create_platform_solvable(module_sack->get_weak_ptr(), platform_id->first, platform_id->second);
            platform_detected = true;
        }
    }

    auto [include_NEVRAs, exclude_NEVRAs, names, src_names, reldep_name_list] = collect_data_for_modular_filtering();

    // Pakages from system, commandline, and hotfix repositories are not targets for modular filterring
    libdnf::rpm::PackageQuery target_packages(base);

    // TODO(replace) "@System", "@commandline" by defined variables like in dnf4
    std::vector<std::string> keep_repo_ids = {"@System", "@commandline"};

    libdnf::repo::RepoQuery hotfix_repos(base);
    hotfix_repos.filter_enabled(true);
    hotfix_repos.filter(
        [](const libdnf::repo::RepoWeakPtr & repo) {
            return repo->get_config().get_module_hotfixes_option().get_value();
        },
        true,
        libdnf::sack::QueryCmp::EQ);
    for (auto & repo : hotfix_repos) {
        keep_repo_ids.push_back(repo->get_id());
    }

    target_packages.filter_repo_id(keep_repo_ids, libdnf::sack::QueryCmp::NEQ);

    libdnf::rpm::PackageQuery include_query(base);
    libdnf::rpm::PackageQuery exclude_query(target_packages);
    libdnf::rpm::PackageQuery exclude_provides_query(target_packages);
    libdnf::rpm::PackageQuery exclude_names_query(target_packages);
    libdnf::rpm::PackageQuery exclude_src_names_query(target_packages);

    include_query.filter_nevra(include_NEVRAs);

    // All packages from not active modules must be filtered out by modular filtering except packages from active
    // modules
    exclude_query.filter_nevra(exclude_NEVRAs);
    exclude_query.difference(include_query);

    // Exclude packages by their Provides. Provides are used to disable obsoletes. Remove included modular packages to
    // not exclude modular packages from active modules
    exclude_provides_query.filter_provides(reldep_name_list);
    exclude_provides_query.difference(include_query);

    // Search for source packages with same names as included source artifacts. Handling of sorce packages differently
    // prevent filtering out of binary packages that has the same name as source package but binary package is not
    // in module (it prevents creation of broken dependenciers in the distribution)
    exclude_src_names_query.filter_name(src_names);
    exclude_src_names_query.filter_arch({"src", "nosrc"});

    // Required to filtrate out source packages and packages with incompatible architectures.
    exclude_names_query.filter_name(names);

    // Performance optimization => merging with exclude_src_names_query will prevent additional removal of included
    // packages. Remove included modular packages to not exclude modular packages from active modules
    exclude_names_query.update(exclude_src_names_query);
    exclude_names_query.difference(include_query);

    base->get_rpm_package_sack()->p_impl->set_module_excludes(exclude_query);
    base->get_rpm_package_sack()->p_impl->add_module_excludes(exclude_provides_query);
    base->get_rpm_package_sack()->p_impl->add_module_excludes(exclude_names_query);

    // TODO(jmracek) Store also includes or data more structuralized - module not actave packages,
    // filtered out not modular packages or so on

    // dnf_sack_set_module_includes(sack, includeQuery.getResultPset());*/
}


void ModuleSack::Impl::make_provides_ready() {
    if (provides_ready) {
        return;
    }

    // Temporarily replaces the considered map with an empty one. Ignores "excludes" during calculation provides.
    Map * considered = pool->considered;
    pool->considered = nullptr;

    // TODO(pkratoch): Internalize repositories

    pool_createwhatprovides(pool);
    provides_ready = true;

    // Sets the original considered map.
    pool->considered = considered;
}


void ModuleSack::Impl::recompute_considered_in_pool() {
    if (considered_uptodate) {
        return;
    }

    // TODO(pkratoch): This can be optimized: pool->considered can be a nullptr if there are no excludes, so, we can
    // check it at the beginning and either set the pool->considered to nullptr or not initialize it in the first
    // place (if it already was a nullptr).
    if (!pool->considered) {
        pool->considered = static_cast<Map *>(g_malloc0(sizeof(Map)));
        map_init(pool->considered, pool->nsolvables);
    } else {
        map_grow(pool->considered, pool->nsolvables);
    }
    map_setall(pool->considered);

    if (excludes) {
        map_subtract(pool->considered, &excludes->get_map());
    }

    considered_uptodate = true;
}


void ModuleSack::Impl::set_active_modules(ModuleGoalPrivate & goal) {
    active_modules.clear();

    std::set<std::string> solvable_names;
    for (auto id : goal.list_installs()) {
        Solvable * s = pool_id2solvable(pool, id);
        const char * name = pool_id2str(pool, s->name);
        solvable_names.emplace(name);
    }
    for (const auto & module_item : modules) {
        std::string solvable_name = module_item->get_name_stream_staticcontext();
        if (solvable_names.contains(solvable_name)) {
            active_modules[module_item->id.id] = module_item.get();
        }
    }
}


std::pair<std::vector<std::vector<std::string>>, ModuleSack::ModuleErrorType> ModuleSack::Impl::module_solve(
    std::vector<ModuleItem *> module_items) {
    std::vector<std::vector<std::string>> problems;
    if (module_items.empty()) {
        active_modules.clear();
        return std::make_pair(problems, ModuleSack::ModuleErrorType::NO_ERROR);
    }

    recompute_considered_in_pool();
    make_provides_ready();

    // Require both enabled and default module streams + require latest versions
    ModuleGoalPrivate goal_strict(base->get_module_sack()->get_weak_ptr());
    // Require only enabled module streams + require latest versions
    ModuleGoalPrivate goal_best(base->get_module_sack()->get_weak_ptr());
    // Require only enabled module streams
    ModuleGoalPrivate goal(base->get_module_sack()->get_weak_ptr());
    // No strict requirements
    ModuleGoalPrivate goal_weak(base->get_module_sack()->get_weak_ptr());

    for (const auto & module_item : module_items) {
        // Create "module(name:stream)" provide reldep
        const Id reldep_id = pool_str2id(pool, fmt::format("module({})", module_item->get_name_stream()).c_str(), 1);
        goal_strict.add_provide_install(reldep_id, 0, 1);
        goal_weak.add_provide_install(reldep_id, 1, 0);
        if (module_db->get_status(module_item->get_name()) == ModuleStatus::ENABLED) {
            goal_best.add_provide_install(reldep_id, 0, 1);
            goal.add_provide_install(reldep_id, 0, 0);
        } else {
            goal_best.add_provide_install(reldep_id, 1, 1);
            goal.add_provide_install(reldep_id, 1, 0);
        }
    }

    auto ret = goal_strict.resolve();

    // TODO(pkratoch): Write debugdata if debug_solver config option is set to true.

    if (ret == libdnf::GoalProblem::NO_PROBLEM) {
        set_active_modules(goal_strict);
        return make_pair(problems, ModuleSack::ModuleErrorType::NO_ERROR);
    }

    // TODO(pkratoch): Get problems
    // problems = goal.describe_all_problem_rules(false);

    ret = goal_best.resolve();

    if (ret == libdnf::GoalProblem::NO_PROBLEM) {
        set_active_modules(goal_best);
        return make_pair(problems, ModuleSack::ModuleErrorType::ERROR_IN_DEFAULTS);
    }

    ret = goal.resolve();

    if (ret == libdnf::GoalProblem::NO_PROBLEM) {
        set_active_modules(goal);
        return make_pair(problems, ModuleSack::ModuleErrorType::ERROR_IN_LATEST);
    }

    // Conflicting modules has to be removed otherwice it could result than one of them will be active
    for (auto conflicting_module_id : goal.list_conflicting()) {
        excludes->add(conflicting_module_id);
    }

    ret = goal_weak.resolve();

    if (ret == libdnf::GoalProblem::NO_PROBLEM) {
        set_active_modules(goal_weak);
        return make_pair(problems, ModuleSack::ModuleErrorType::ERROR);
    }

    auto logger = base->get_logger();
    logger->critical("Modularity filtering totally broken\n");

    active_modules.clear();
    return make_pair(problems, ModuleSack::ModuleErrorType::CANNOT_RESOLVE_MODULES);
}


/// @brief Parse platform id from string value.
/// @param platform_id Platform id string.
/// @return Pair where first item is the platform module name and second is the platform stream.
static std::pair<std::string, std::string> parse_platform_id_from_string(const std::string & platform_id) {
    auto index = platform_id.find(':');
    if (index == std::string::npos) {
        return {};
    } else {
        return make_pair(platform_id.substr(0, index), platform_id.substr(index + 1));
    }
}


/// @brief Custom exception to recognize an error in platform id string format.
class PlatformIdFormatError : public libdnf::Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::module"; }
    const char * get_name() const noexcept override { return "PlatformIdFormatError"; }
};


/// @brief Parse platform id from string value inside the 'os-release' file.
/// @param os_release_path Full path to the 'os-release' file.
/// @return Pair where first item is the platform module name and second is the platform stream.
/// @throws PlatformIdFormatError when platform id in the file has incorrect format.
/// @throws std::filesystem::filesystem_error when there is any error during reading a file.
static std::pair<std::string, std::string> parse_platform_id_from_file(const std::string & os_release_path) {
    libdnf::utils::fs::File file{os_release_path, "r"};
    std::string line;
    while (file.read_line(line)) {
        if (line.find("PLATFORM_ID") != std::string::npos) {
            auto eq_position = line.find('=');
            if (eq_position == std::string::npos) {
                throw PlatformIdFormatError(M_("Missing '='"));
            }
            auto start_position = line.find('"', eq_position + 1);
            if (start_position == std::string::npos) {
                throw PlatformIdFormatError(M_("Missing '\"' in the value"));
            }
            auto colon_position = line.find(':', eq_position + 1);
            if (colon_position == std::string::npos) {
                throw PlatformIdFormatError(M_("Missing ':' in the value"));
            }
            auto end_position = line.find('"', colon_position + 1);
            if (end_position == std::string::npos) {
                throw PlatformIdFormatError(M_("Missing '\"' in the value"));
            }

            return make_pair(
                line.substr(start_position + 1, colon_position - start_position - 1),
                line.substr(colon_position + 1, end_position - colon_position - 1));
        }
    }
    return {};
}


/// @brief Parse value inside parentheses of the provide string value.
/// Example of a specific usecase:
/// - when the \a provide_name is 'base-module'.
/// - and the whole provides string is 'base-module(platform:f38)'.
/// - then the parsed value is 'platform:f38'.
/// @param packages Package set where the provides strings are searched for.
/// @param provide_name Name of the provide to be parsed.
/// @return Set of all parsed values found in \a packages from provides having the \a provide_name name.
static std::set<std::string> get_strings_from_provide(
    const libdnf::rpm::PackageSet & packages, const char * provide_name) {
    std::set<std::string> strings;

    auto provide_name_length = strlen(provide_name);
    for (auto const & package : packages) {
        auto const & provides = package.get_provides();
        auto found_provide = std::find_if(provides.begin(), provides.end(), [&](const libdnf::rpm::Reldep & dep) {
            return !strncmp(dep.get_name(), provide_name, strlen(provide_name));
        });
        if (found_provide != provides.end()) {
            auto found_provide_name = (*found_provide).get_name();
            auto found_provide_name_length = strlen(found_provide_name);
            if (found_provide_name[provide_name_length] == '(' &&
                found_provide_name[found_provide_name_length - 1] == ')') {
                strings.emplace(
                    found_provide_name + provide_name_length + 1, found_provide_name_length - provide_name_length - 2);
            }
        }
    }

    return strings;
}


std::optional<std::pair<std::string, std::string>> ModuleSack::Impl::detect_platform_name_and_stream() const {
    // try to detect platform id from configuration
    auto & config_platform_option = base->get_config().get_module_platform_id_option();
    if (!config_platform_option.empty()) {
        auto & config_platform = config_platform_option.get_value();
        auto parsed_platform_id = parse_platform_id_from_string(config_platform);
        if (!parsed_platform_id.first.empty() && !parsed_platform_id.second.empty()) {
            return parsed_platform_id;
        } else {
            base->get_logger()->debug("Invalid format of platform ID in configuration: {}", config_platform);
        }
    }

    libdnf::rpm::PackageQuery base_query(base);
    base_query.filter_provides({"system-release"});
    base_query.filter_latest_evr();

    // try to detect platform id from available packages
    libdnf::rpm::PackageQuery available_query(base_query);
    available_query.filter_available();
    auto available_provides = get_strings_from_provide(available_query, "base-module");
    if (available_provides.size() == 1) {
        auto & query_platform_id = *available_provides.begin();
        auto parsed_platform_id = parse_platform_id_from_string(query_platform_id);
        if (!parsed_platform_id.first.empty() && !parsed_platform_id.second.empty()) {
            return parsed_platform_id;
        } else {
            base->get_logger()->debug(
                "Invalid format of module platform queried from available packages: {}", query_platform_id);
        }
    } else {
        base->get_logger()->debug("Multiple module platforms provided by available packages");
    }

    // try to detect platform id from installed packages
    libdnf::rpm::PackageQuery installed_query(base_query);
    installed_query.filter_installed();
    auto installed_provides = get_strings_from_provide(installed_query, "base-module");
    if (installed_provides.size() == 1) {
        auto & query_platform_id = *installed_provides.begin();
        auto parsed_platform_id = parse_platform_id_from_string(query_platform_id);
        if (!parsed_platform_id.first.empty() && !parsed_platform_id.second.empty()) {
            return parsed_platform_id;
        } else {
            base->get_logger()->debug(
                "Invalid format of module platform queried from installed packages: {}", query_platform_id);
        }
    } else {
        base->get_logger()->debug("Multiple module platforms provided by installed packages");
    }

    // try to detect platform id from release files
    // TODO(jkolarik): Create constants for paths
    std::vector<std::string> os_release_paths{"etc/os-release", "usr/lib/os-release"};
    auto & installroot = base->get_config().get_installroot_option().get_value();
    for (auto & os_release_path : os_release_paths) {
        std::string full_path = std::filesystem::canonical(installroot) / os_release_path;
        std::pair<std::string, std::string> parsed_platform_id;

        try {
            parsed_platform_id = parse_platform_id_from_file(full_path);
        } catch (const PlatformIdFormatError & id_except) {
            base->get_logger()->debug(
                "Invalid format of PLATFORM_ID in '{}': {}", full_path, std::string(id_except.what()));
        } catch (const std::filesystem::filesystem_error & fs_except) {
            base->get_logger()->debug(
                "Detection of module platform in '{}' failed: {}", full_path, std::string(fs_except.what()));
        }

        if (!parsed_platform_id.first.empty() && !parsed_platform_id.second.empty()) {
            return parsed_platform_id;
        }
    }

    base->get_logger()->debug("No valid Platform ID detected");
    return std::nullopt;
}


std::pair<std::vector<std::vector<std::string>>, ModuleSack::ModuleErrorType>
ModuleSack::resolve_active_module_items() {
    p_impl->considered_uptodate = false;
    p_impl->excludes.reset(new libdnf::solv::SolvMap(p_impl->pool->nsolvables));
    p_impl->module_db->initialize();

    ModuleStatus status;
    std::vector<ModuleItem *> module_items_to_solve;
    // Use only enabled or default modules for transaction
    for (const auto & module_item : get_modules()) {
        const auto & module_name = module_item->get_name();
        status = p_impl->module_db->get_status(module_name);
        if (status == ModuleStatus::DISABLED) {
            p_impl->excludes->add(module_item->id.id);
        } else if (
            status == ModuleStatus::ENABLED &&
            p_impl->module_db->get_enabled_stream(module_name) == module_item->get_stream()) {
            module_items_to_solve.push_back(module_item.get());
        } else if (status == ModuleStatus::AVAILABLE && get_default_stream(module_name) == module_item->get_stream()) {
            module_items_to_solve.push_back(module_item.get());
        }
    }

    auto problems = p_impl->module_solve(module_items_to_solve);
    active_modules_resolved = true;
    return problems;
}


InvalidModuleStatus::InvalidModuleStatus(const std::string & status)
    : libdnf::Error(M_("Invalid module status: {}"), status) {}


std::string module_status_to_string(ModuleStatus status) {
    switch (status) {
        case ModuleStatus::AVAILABLE:
            return "Available";
        case ModuleStatus::ENABLED:
            return "Enabled";
        case ModuleStatus::DISABLED:
            return "Disabled";
    }
    return "";
}


ModuleStatus module_status_from_string(const std::string & status) {
    if (status == "Available") {
        return ModuleStatus::AVAILABLE;
    } else if (status == "Enabled") {
        return ModuleStatus::ENABLED;
    } else if (status == "Disabled") {
        return ModuleStatus::DISABLED;
    }

    throw InvalidModuleStatus(status);
}

}  // namespace libdnf::module
