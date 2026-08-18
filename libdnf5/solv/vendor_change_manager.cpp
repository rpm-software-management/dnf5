// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "vendor_change_manager.hpp"

#include "pool.hpp"
#include "utils/fs/utils.hpp"
#include "utils/string.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/sack/match_string.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <toml.hpp>

#include <array>
#include <map>
#include <optional>
#include <string_view>
#include <utility>

namespace libdnf5::solv {

namespace {

// supported config file version
constexpr std::array<std::string_view, 3> CONF_FILE_SUPPORTED_VERSIONS = {"1.0", "1.1", "1.2"};

const std::map<std::string_view, sack::QueryCmp> COMPARATORS = {
    {"EXACT", sack::QueryCmp::EXACT},
    {"NOT_EXACT", sack::QueryCmp::NOT_EXACT},
    {"IEXACT", sack::QueryCmp::IEXACT},
    {"NOT_IEXACT", sack::QueryCmp::NOT_IEXACT},
    {"CONTAINS", sack::QueryCmp::CONTAINS},
    {"NOT_CONTAINS", sack::QueryCmp::NOT_CONTAINS},
    {"ICONTAINS", sack::QueryCmp::ICONTAINS},
    {"NOT_ICONTAINS", sack::QueryCmp::NOT_ICONTAINS},
    {"STARTSWITH", sack::QueryCmp::STARTSWITH},
    {"ISTARTSWITH", sack::QueryCmp::ISTARTSWITH},
    {"ENDSWITH", sack::QueryCmp::ENDSWITH},
    {"IENDSWITH", sack::QueryCmp::IENDSWITH},
    {"REGEX", sack::QueryCmp::REGEX},
    {"IREGEX", sack::QueryCmp::IREGEX},
    {"GLOB", sack::QueryCmp::GLOB},
    {"NOT_GLOB", sack::QueryCmp::NOT_GLOB},
    {"IGLOB", sack::QueryCmp::IGLOB},
    {"NOT_IGLOB", sack::QueryCmp::NOT_IGLOB},

    {"GT", sack::QueryCmp::GT},
    {"GTE", sack::QueryCmp::GTE},
    {"LT", sack::QueryCmp::LT},
    {"LTE", sack::QueryCmp::LTE},
};

const std::map<std::string_view, sack::QueryCmp> COMPARATORS_1_2 = {
    {"NOT_STARTSWITH", sack::QueryCmp::NOT | sack::QueryCmp::STARTSWITH},
    {"NOT_ISTARTSWITH", sack::QueryCmp::NOT | sack::QueryCmp::ISTARTSWITH},
    {"NOT_ENDSWITH", sack::QueryCmp::NOT | sack::QueryCmp::ENDSWITH},
    {"NOT_IENDSWITH", sack::QueryCmp::NOT | sack::QueryCmp::IENDSWITH},
    {"NOT_REGEX", sack::QueryCmp::NOT | sack::QueryCmp::REGEX},
    {"NOT_IREGEX", sack::QueryCmp::NOT | sack::QueryCmp::IREGEX},
};


class VendorChangePolicyConfigFileError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::solv"; }
    const char * get_name() const noexcept override { return "VendorChangePolicyConfigFileError"; }
};


bool is_string_comparator(sack::QueryCmp cmp) {
    auto base = cmp - sack::QueryCmp::NOT - sack::QueryCmp::ICASE;
    return base == sack::QueryCmp::EXACT || base == sack::QueryCmp::CONTAINS || base == sack::QueryCmp::STARTSWITH ||
           base == sack::QueryCmp::ENDSWITH || base == sack::QueryCmp::REGEX || base == sack::QueryCmp::GLOB;
}


bool is_relational_comparator(sack::QueryCmp cmp) {
    return cmp == sack::QueryCmp::EXACT || cmp == sack::QueryCmp::NOT_EXACT || cmp == sack::QueryCmp::GT ||
           cmp == sack::QueryCmp::GTE || cmp == sack::QueryCmp::LT || cmp == sack::QueryCmp::LTE;
}


sack::QueryCmp string_to_comparator(
    const std::string & str_comparator,
    const std::string & cfg_version,
    const std::filesystem::path & path,
    std::size_t line_num) {
    if (const auto it = COMPARATORS.find(str_comparator); it != COMPARATORS.end()) {
        return it->second;
    }
    const auto it_12 = COMPARATORS_1_2.find(str_comparator);
    if (it_12 == COMPARATORS_1_2.end()) {
        throw VendorChangePolicyConfigFileError(
            M_("Unknown 'comparator' \"{}\" in file \"{}\" on line {}"), str_comparator, path.native(), line_num);
    }
    if (cfg_version != "1.2") {
        throw VendorChangePolicyConfigFileError(
            M_("Configuration file \"{}\" uses version \"{}\" which does not support 'comparator' \"{}\""),
            path.native(),
            cfg_version,
            str_comparator);
    }
    return it_12->second;
}


std::string comparator_to_string(sack::QueryCmp comparator) {
    for (const auto & [text, cmp_value] : COMPARATORS) {
        if (comparator == cmp_value) {
            return std::string{text};
        }
    }
    for (const auto & [text, cmp_value] : COMPARATORS_1_2) {
        if (comparator == cmp_value) {
            return std::string{text};
        }
    }
    libdnf_throw_assertion("Invalid value of comparator");
}


#ifdef TOML11_COMPAT
inline auto location_first_line_num(const toml::source_location & location) {
    return location.line();
}
#else   // #ifdef TOML11_COMPAT
inline auto location_first_line_num(const toml::source_location & location) {
    return location.first_line_number();
}
#endif  // #ifdef TOML11_COMPAT

}  // namespace


VendorChangeManager::VendorChangeManager(const Pool & pool) : pool{pool} {}


bool VendorChangeManager::is_vendor_change_allowed(Solvable & outgoing, Solvable & incoming) {
    // Treat a missing vendor as an empty string (ID_EMPTY)
    auto outgoing_vendor = outgoing.vendor ? outgoing.vendor : ID_EMPTY;
    auto incoming_vendor = incoming.vendor ? incoming.vendor : ID_EMPTY;

    if (incoming_vendor == outgoing_vendor) {
        return true;  // OK, no vendor change occurred
    }

    // Check if the incoming solvable bypasses vendor check.
    // If so, always accept the vendor change without further checks.
    if (is_incoming_vendor_bypassed_solvable(pool_solvable2id(*pool, &incoming))) {
        return true;
    }

    const auto & outgoing_vendor_mask = get_vendor_change_masks(outgoing_vendor).outgoing_mask;
    if (outgoing_vendor_mask.empty()) {
        // The outgoing vendor is not involved in any valid policy change.
        // Therefore, any change is illegal.
        return false;
    }

    const auto & incoming_vendor_mask = get_vendor_change_masks(incoming_vendor).incoming_mask;

    // The resulting mask should only contain vendors present in both input masks (intersection).
    // SolvMap::operator&= requires the LHS mask size to be less than or equal to the RHS mask size.
    solv::SolvMap result_vendor_mask{0};
    if (outgoing_vendor_mask.allocated_size() > incoming_vendor_mask.allocated_size()) {
        result_vendor_mask = incoming_vendor_mask;
        result_vendor_mask &= outgoing_vendor_mask;
    } else {
        result_vendor_mask = outgoing_vendor_mask;
        result_vendor_mask &= incoming_vendor_mask;
    }

    // Iterate through policies allowing vendor changes and verify
    // if the change is permitted for the specific 'installed' and 'new_solv' packages.
    for (const auto policy_idx : result_vendor_mask) {
        const auto & policy = vendor_policies_def.at(static_cast<unsigned int>(policy_idx));

        // Ensure the current installed package is allowed to leave its vendor
        if (!matches_package_defs(policy.outgoing_packages, outgoing)) {
            continue;
        }

        // Ensure the new package is allowed to be accepted by the new vendor
        if (matches_package_defs(policy.incoming_packages, incoming)) {
            return true;  // Vendor change is allowed
        }
    }

    return false;  // Illegal vendor change
}


const VendorChangeManager::VendorChangeMasks & VendorChangeManager::get_vendor_change_masks(Id vendor) {
    constexpr int EXTRA_CAPACITY = 7;
    static const VendorChangeMasks empty_masks{.vendor = 0};
    VendorChangeMasks masks;

    if (vendor == 0 || vendor_policies_def.empty()) {
        return empty_masks;
    }

    for (const auto & vendor_to_classes : vendor_masks) {
        if (vendor_to_classes.vendor == vendor) {
            return vendor_to_classes;
        }
    }

    masks.vendor = vendor;
    auto vendor_str = pool.id2str(vendor);
    for (unsigned int class_idx = 0; class_idx < vendor_policies_def.size(); ++class_idx) {
        const auto & vendor_class_def = vendor_policies_def[class_idx];
        using VendorGroupType = VendorChangePolicy::VendorGroupType;
        bool has_outgoing = false;
        bool has_incoming = false;
        bool outgoing_resolved = false;
        bool incoming_resolved = false;
        for (const auto & entry : vendor_class_def.vendor_entries) {
            const bool is_outgoing =
                entry.group_type == VendorGroupType::OUTGOING || entry.group_type == VendorGroupType::EQUIVALENT;
            const bool is_incoming =
                entry.group_type == VendorGroupType::INCOMING || entry.group_type == VendorGroupType::EQUIVALENT;
            has_outgoing |= is_outgoing;
            has_incoming |= is_incoming;
            if (sack::match_string(vendor_str, entry.def.comparator, entry.def.vendor)) {
                if (is_outgoing && !outgoing_resolved) {
                    outgoing_resolved = true;
                    if (!entry.def.is_exclusion) {
                        masks.outgoing_mask.add_grow(static_cast<int>(class_idx), EXTRA_CAPACITY);
                    }
                }
                if (is_incoming && !incoming_resolved) {
                    incoming_resolved = true;
                    if (!entry.def.is_exclusion) {
                        masks.incoming_mask.add_grow(static_cast<int>(class_idx), EXTRA_CAPACITY);
                    }
                }
            }
            if (outgoing_resolved && incoming_resolved) {
                break;
            }
        }
        if (!has_outgoing) {
            // Default to permit if no specific outgoing vendors are defined
            masks.outgoing_mask.add_grow(static_cast<int>(class_idx), EXTRA_CAPACITY);
        }
        if (!has_incoming) {
            // Default to permit if no specific incoming vendors are defined
            masks.incoming_mask.add_grow(static_cast<int>(class_idx), EXTRA_CAPACITY);
        }
    }

    return vendor_masks.emplace_back(masks);
}


bool VendorChangeManager::matches_package_defs(
    const std::vector<VendorChangeManager::VendorChangePolicy::PackageDef> & pkgs_def, const Solvable & solvable) {
    if (pkgs_def.empty()) {
        return true;  // Default to true if no specific policies are defined
    }

    for (const auto & pkg_def : pkgs_def) {
        bool pass_filters = true;
        for (const auto & filter : pkg_def.filters) {
            if (!filter.filter_func(pool, solvable, filter)) {
                pass_filters = false;
                break;
            }
        }

        // If all filters in this definition match the solvable
        if (pass_filters) {
            return !pkg_def.is_exclusion;  // Return true unless this is an exclusion rule
        }
    }

    return false;  // Disallow change if no rules matched (Allowlist principle)
}


namespace {

bool filter_package_name(
    const Pool & pool,
    const Solvable & solvable,
    const VendorChangeManager::VendorChangePolicy::PackageDef::Filter & filter) {
    const auto value = libdnf5::utils::string::c_to_str(pool.id2str(solvable.name));
    return sack::match_string(value, filter.comparator, filter.value);
}


bool filter_package_source_name(
    const Pool & pool,
    const Solvable & solvable,
    const VendorChangeManager::VendorChangePolicy::PackageDef::Filter & filter) {
    const char * source_name =
        pool.lookup_str(pool.solvable2id(const_cast<Solvable *>(&solvable)), SOLVABLE_SOURCENAME);
    if (!source_name) {
        source_name = pool.id2str(solvable.name);
    }
    const auto value = libdnf5::utils::string::c_to_str(source_name);
    return sack::match_string(value, filter.comparator, filter.value);
}


bool filter_package_evr(
    const Pool & pool,
    const Solvable & solvable,
    const VendorChangeManager::VendorChangePolicy::PackageDef::Filter & filter) {
    const auto value = libdnf5::utils::string::c_to_str(pool.id2str(solvable.evr));
    switch (filter.comparator) {
        case sack::QueryCmp::GT:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_COMPARE) > 0;
        case sack::QueryCmp::GTE:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_COMPARE) >= 0;
        case sack::QueryCmp::LT:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_COMPARE) < 0;
        case sack::QueryCmp::LTE:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_COMPARE) <= 0;
        default:
            return sack::match_string(value, filter.comparator, filter.value);
    }
}


bool filter_package_epoch(
    const Pool & pool,
    const Solvable & solvable,
    const VendorChangeManager::VendorChangePolicy::PackageDef::Filter & filter) {
    const auto value = pool.get_epoch_num(pool.solvable2id(const_cast<Solvable *>(&solvable)));
    const auto pattern = std::stoul(filter.value);
    switch (filter.comparator) {
        case sack::QueryCmp::EXACT:
            return value == pattern;
        case sack::QueryCmp::NOT_EXACT:
            return value != pattern;
        case sack::QueryCmp::GT:
            return value > pattern;
        case sack::QueryCmp::GTE:
            return value >= pattern;
        case sack::QueryCmp::LT:
            return value < pattern;
        case sack::QueryCmp::LTE:
            return value <= pattern;
        default:
            return true;
    }
}


bool filter_package_version(
    const Pool & pool,
    const Solvable & solvable,
    const VendorChangeManager::VendorChangePolicy::PackageDef::Filter & filter) {
    const auto value = libdnf5::utils::string::c_to_str(pool.split_evr(pool.id2str(solvable.evr)).v);
    switch (filter.comparator) {
        case sack::QueryCmp::GT:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_COMPARE_EVONLY) > 0;
        case sack::QueryCmp::GTE:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_COMPARE_EVONLY) >= 0;
        case sack::QueryCmp::LT:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_COMPARE_EVONLY) < 0;
        case sack::QueryCmp::LTE:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_COMPARE_EVONLY) <= 0;
        default:
            return sack::match_string(value, filter.comparator, filter.value);
    }
}


bool filter_package_release(
    const Pool & pool,
    const Solvable & solvable,
    const VendorChangeManager::VendorChangePolicy::PackageDef::Filter & filter) {
    const auto value = '-' + libdnf5::utils::string::c_to_str(pool.split_evr(pool.id2str(solvable.evr)).r);
    switch (filter.comparator) {
        case sack::QueryCmp::GT:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_MATCH_RELEASE) > 0;
        case sack::QueryCmp::GTE:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_MATCH_RELEASE) >= 0;
        case sack::QueryCmp::LT:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_MATCH_RELEASE) < 0;
        case sack::QueryCmp::LTE:
            return pool.evrcmp_str(value.c_str(), filter.value.c_str(), EVRCMP_MATCH_RELEASE) <= 0;
        default:
            return sack::match_string(value, filter.comparator, filter.value);
    }
}


bool filter_package_arch(
    const Pool & pool,
    const Solvable & solvable,
    const VendorChangeManager::VendorChangePolicy::PackageDef::Filter & filter) {
    const auto value = libdnf5::utils::string::c_to_str(pool.id2str(solvable.arch));
    return sack::match_string(value, filter.comparator, filter.value);
}


bool filter_package_repoid(
    const Pool & pool,
    const Solvable & solvable,
    const VendorChangeManager::VendorChangePolicy::PackageDef::Filter & filter) {
    const auto value = pool.get_repo(pool.solvable2id(const_cast<Solvable *>(&solvable))).get_id();
    return sack::match_string(value, filter.comparator, filter.value);
}


bool filter_package_cmdline_repo(
    const Pool & pool,
    const Solvable & solvable,
    const VendorChangeManager::VendorChangePolicy::PackageDef::Filter & filter) {
    const auto repo_type = pool.get_repo(pool.solvable2id(const_cast<Solvable *>(&solvable))).get_type();
    // filter.value is a string-encoded bool (empty string == false)
    // filter.comparator is unused for this specific filter type
    return (repo_type == repo::Repo::Type::COMMANDLINE) != filter.value.empty();
}


VendorChangeManager::VendorChangePolicy::PackageDef::Filter read_package_def_filter(
    const toml::value & filter_table, const std::filesystem::path & path, const std::string & cfg_version) {
    VendorChangeManager::VendorChangePolicy::PackageDef::Filter filter;
    filter.comparator = sack::QueryCmp::EXACT;

    bool filter_found = false;
    bool value_found = false;
    std::string filter_str;
    for (const auto & [key, value] : filter_table.as_table()) {
        if (key == "filter") {
            filter_str = value.as_string();
            if (filter_str == "name") {
                filter.filter_func = filter_package_name;
            } else if (filter_str == "source_name") {
                filter.filter_func = filter_package_source_name;
            } else if (filter_str == "evr") {
                filter.filter_func = filter_package_evr;
            } else if (filter_str == "epoch") {
                filter.filter_func = filter_package_epoch;
            } else if (filter_str == "version") {
                filter.filter_func = filter_package_version;
            } else if (filter_str == "release") {
                filter.filter_func = filter_package_release;
            } else if (filter_str == "arch") {
                filter.filter_func = filter_package_arch;
            } else if (filter_str == "repoid") {
                filter.filter_func = filter_package_repoid;
            } else if (filter_str == "cmdline_repo") {
                filter.filter_func = filter_package_cmdline_repo;
            } else {
                const auto location = value.location();
                throw VendorChangePolicyConfigFileError(
                    M_("Unknown 'filter' \"{}\" in file \"{}\" on line {}"),
                    filter_str,
                    path.native(),
                    location_first_line_num(location));
            }
            filter_found = true;
        } else if (key == "value") {
            filter.value = value.as_string();
            value_found = true;
        } else if (key == "comparator") {
            const auto line_num = location_first_line_num(value.location());
            filter.comparator = string_to_comparator(value.as_string(), cfg_version, path, line_num);
        } else {
            const auto location = value.location();
            throw VendorChangePolicyConfigFileError(
                M_("Unknown key '{}' in file \"{}\" on line {}"),
                key,
                path.native(),
                location_first_line_num(location));
        }
    }
    if (!filter_found) {
        const auto location = filter_table.location();
        throw VendorChangePolicyConfigFileError(
            M_("Missing 'filter' key in file \"{}\" for table entry on line {}"),
            path.native(),
            location_first_line_num(location));
    }
    if (!value_found) {
        const auto location = filter_table.location();
        throw VendorChangePolicyConfigFileError(
            M_("Missing 'value' key in file \"{}\" for table entry on line {}"),
            path.native(),
            location_first_line_num(location));
    }

    if (filter.filter_func == filter_package_cmdline_repo) {
        if (filter.comparator != sack::QueryCmp::EXACT) {
            const auto location = filter_table.location();
            throw VendorChangePolicyConfigFileError(
                M_("Filter \"cmdline_repo\" in file \"{}\" in table starting on line {} "
                   "does not support comparator \"{}\"."
                   " Only the default \"EXACT\" comparator is allowed for this filter"),
                path.native(),
                location_first_line_num(location),
                comparator_to_string(filter.comparator));
        }

        // Optimization: bool value is stored in a string (empty string == false).
        // This avoids expensive string comparisons during filtering
        if (filter.value == "1" || filter.value == "true") {
            std::string{"1"}.swap(filter.value);  // store non-empty string
        } else if (filter.value == "0" || filter.value == "false") {
            std::string{}.swap(filter.value);  // store empty string
        } else {
            const auto location = filter_table.location();
            throw VendorChangePolicyConfigFileError(
                M_("Invalid 'value' \"{}\" in file \"{}\" in table starting on line {}."
                   " Only \"true\", \"1\", \"false\", \"0\" are supported"),
                filter.value,
                path.native(),
                location_first_line_num(location));
        }
    } else if (
        filter.filter_func == filter_package_evr || filter.filter_func == filter_package_epoch ||
        filter.filter_func == filter_package_version || filter.filter_func == filter_package_release) {
        if (!is_relational_comparator(filter.comparator)) {
            const auto location = filter_table.location();
            throw VendorChangePolicyConfigFileError(
                M_("Filter \"{}\" in file \"{}\" in table starting on line {} "
                   "does not support comparator \"{}\""),
                filter_str,
                path.native(),
                location_first_line_num(location),
                comparator_to_string(filter.comparator));
        }
    } else {
        if (!is_string_comparator(filter.comparator)) {
            const auto location = filter_table.location();
            throw VendorChangePolicyConfigFileError(
                M_("Filter \"{}\" in file \"{}\" in table starting on line {} "
                   "does not support comparator \"{}\""),
                filter_str,
                path.native(),
                location_first_line_num(location),
                comparator_to_string(filter.comparator));
        }
    }

    // Validate regex pattern
    if (const auto cmp = filter.comparator - sack::QueryCmp::NOT - sack::QueryCmp::ICASE;
        cmp == sack::QueryCmp::REGEX) {
        try {
            sack::match_string("", filter.comparator, filter.value);
        } catch (const std::exception & ex) {
            const auto location = filter_table.location();
            throw VendorChangePolicyConfigFileError(
                M_("Invalid regex \"{}\" in file \"{}\" in table starting on line {}: {}"),
                filter.value,
                path.native(),
                location_first_line_num(location),
                std::string(ex.what()));
        }
    }

    return filter;
}

}  // namespace


void VendorChangeManager::load_vendor_change_policy(const std::filesystem::path & path) {
    VendorChangePolicy policy;

    bool is_config_version_1_0;

    try {
        // Parse the TOML file
        auto config = toml::parse(path);

        // Check config file version
        const auto version = toml::find<std::optional<std::string>>(config, "version");
        if (!version) {
            throw VendorChangePolicyConfigFileError(M_("Missing \"version\" key in file \"{}\""), path.native());
        }

        bool is_supported_version{false};
        for (const auto supported_version : CONF_FILE_SUPPORTED_VERSIONS) {
            if (version == supported_version) {
                is_supported_version = true;
                break;
            }
        }
        if (!is_supported_version) {
            throw VendorChangePolicyConfigFileError(
                M_("Unsupported version \"{}\" in file \"{}\". Supported versions: {}"),
                *version,
                path.native(),
                libdnf5::utils::string::join(CONF_FILE_SUPPORTED_VERSIONS, ", "));
        }

        is_config_version_1_0 = version == "1.0";

        if (is_config_version_1_0 && config.contains("equivalent_vendors") &&
            (config.contains("outgoing_vendors") || config.contains("incoming_vendors"))) {
            throw VendorChangePolicyConfigFileError(
                M_("Configuration file \"{}\" uses version \"1.0\" which does not support combining"
                   " 'equivalent_vendors' with 'outgoing_vendors' and 'incoming_vendors'"),
                path.native());
        }

        for (const auto & [element, value] : config.as_table()) {
            if (element == "version") {
                continue;
            }

            enum class GroupType {
                OUTGOING_VENDORS,
                INCOMING_VENDORS,
                EQUIVALENT_VENDORS,
                OUTGOING_PACKAGES,
                INCOMING_PACKAGES
            };

            GroupType group_type;
            if (element == "outgoing_vendors") {
                group_type = GroupType::OUTGOING_VENDORS;
            } else if (element == "incoming_vendors") {
                group_type = GroupType::INCOMING_VENDORS;
            } else if (element == "equivalent_vendors") {
                group_type = GroupType::EQUIVALENT_VENDORS;
            } else if (element == "outgoing_packages") {
                group_type = GroupType::OUTGOING_PACKAGES;
            } else if (element == "incoming_packages") {
                group_type = GroupType::INCOMING_PACKAGES;
            } else {
                const auto location = value.location();
                throw VendorChangePolicyConfigFileError(
                    M_("Unknown key '{}' in file \"{}\" on line {}"),
                    element,
                    path.native(),
                    location_first_line_num(location));
            }

            if (group_type == GroupType::OUTGOING_PACKAGES || group_type == GroupType::INCOMING_PACKAGES) {
                if (is_config_version_1_0) {
                    const auto location = value.location();
                    throw VendorChangePolicyConfigFileError(
                        M_("Configuration file \"{}\" uses version \"1.0\""
                           " which does not support key '{}' on line {}"),
                        path.native(),
                        element,
                        location_first_line_num(location));
                }

                for (const auto & entry : value.as_array()) {
                    VendorChangePolicy::PackageDef package_def;
                    package_def.is_exclusion = false;

                    for (const auto & [key, value] : entry.as_table()) {
                        if (key == "filters") {
                            for (const auto & filter_entry : value.as_array()) {
                                auto filter = read_package_def_filter(filter_entry, path, *version);
                                if (filter.filter_func == filter_package_cmdline_repo &&
                                    group_type == GroupType::OUTGOING_PACKAGES) {
                                    const auto location = filter_entry.location();
                                    throw VendorChangePolicyConfigFileError(
                                        M_("Filter \"cmdline_repo\" is only allowed in the 'incoming_packages' section."
                                           " Error in file \"{}\" in table starting on line {}"),
                                        path.native(),
                                        location_first_line_num(location));
                                }
                                package_def.filters.emplace_back(std::move(filter));
                            }
                        } else if (key == "exclude") {
                            package_def.is_exclusion = value.as_boolean();
                        } else {
                            const auto location = value.location();
                            throw VendorChangePolicyConfigFileError(
                                M_("Unknown key '{}' in file \"{}\" on line {}"),
                                key,
                                path.native(),
                                location_first_line_num(location));
                        }
                    }

                    if (package_def.filters.empty()) {
                        const auto location = entry.location();
                        throw VendorChangePolicyConfigFileError(
                            M_("Missing package filter definition in file \"{}\" in table starting on line {}"),
                            path.native(),
                            location_first_line_num(location));
                    }

                    if (group_type == GroupType::OUTGOING_PACKAGES) {
                        policy.outgoing_packages.emplace_back(std::move(package_def));
                    } else {
                        policy.incoming_packages.emplace_back(std::move(package_def));
                    }
                }

                continue;
            }

            for (const auto & entry : value.as_array()) {
                VendorChangePolicy::VendorDef vendor_def;
                vendor_def.comparator = sack::QueryCmp::EXACT;
                vendor_def.is_exclusion = false;

                bool is_vendor_set = false;
                for (const auto & [key, value] : entry.as_table()) {
                    if (key == "vendor") {
                        vendor_def.vendor = value.as_string();
                        is_vendor_set = true;
                    } else if (key == "comparator") {
                        const auto line_num = location_first_line_num(value.location());
                        const auto comparator = string_to_comparator(value.as_string(), *version, path, line_num);
                        if (!is_string_comparator(comparator)) {
                            const auto location = value.location();
                            throw VendorChangePolicyConfigFileError(
                                M_("Unsupported comparator \"{}\" for vendor definition in file \"{}\" on line {}"),
                                comparator_to_string(comparator),
                                path.native(),
                                location_first_line_num(location));
                        }
                        vendor_def.comparator = comparator;
                    } else if (key == "exclude") {
                        vendor_def.is_exclusion = value.as_boolean();
                    } else {
                        const auto location = value.location();
                        throw VendorChangePolicyConfigFileError(
                            M_("Unknown key '{}' in file \"{}\" on line {}"),
                            key,
                            path.native(),
                            location_first_line_num(location));
                    }
                }

                if (!is_vendor_set) {
                    const auto location = entry.location();
                    throw VendorChangePolicyConfigFileError(
                        M_("Missing 'vendor' key in file \"{}\" in table starting on line {}"),
                        path.native(),
                        location_first_line_num(location));
                }

                // Validate regex pattern
                if (const auto cmp = vendor_def.comparator - sack::QueryCmp::NOT - sack::QueryCmp::ICASE;
                    cmp == sack::QueryCmp::REGEX) {
                    try {
                        sack::match_string("", vendor_def.comparator, vendor_def.vendor);
                    } catch (const std::exception & ex) {
                        const auto location = entry.location();
                        throw VendorChangePolicyConfigFileError(
                            M_("Invalid regex vendor pattern \"{}\" in file \"{}\" in table starting on line {}: {}"),
                            vendor_def.vendor,
                            path.native(),
                            location_first_line_num(location),
                            std::string(ex.what()));
                    }
                }

                using VendorGroupType = VendorChangePolicy::VendorGroupType;
                VendorGroupType vgt;
                switch (group_type) {
                    case GroupType::OUTGOING_VENDORS:
                        vgt = VendorGroupType::OUTGOING;
                        break;
                    case GroupType::INCOMING_VENDORS:
                        vgt = VendorGroupType::INCOMING;
                        break;
                    case GroupType::EQUIVALENT_VENDORS:
                        vgt = VendorGroupType::EQUIVALENT;
                        break;
                    default:
                        libdnf_throw_assertion("Invalid vendor change policy direction");
                }
                policy.vendor_entries.emplace_back(vgt, std::move(vendor_def));
            }
        }
    } catch (const toml::type_error & ex) {
        auto loc = ex.location();
        throw VendorChangePolicyConfigFileError(
            M_("Bad value type in file \"{}\" on line {}: {}"),
            path.native(),
            location_first_line_num(loc),
            std::string(ex.what()));
    } catch (const toml::exception & ex) {
        throw VendorChangePolicyConfigFileError(
            M_("An error occurred when parsing file \"{}\": {}"), path.native(), std::string(ex.what()));
    }

    if (policy.outgoing_packages.empty() && policy.incoming_packages.empty() && policy.vendor_entries.empty()) {
        // All lists are empty, so there is nothing to add
        return;
    }

    if (is_config_version_1_0) {
        using VendorGroupType = VendorChangePolicy::VendorGroupType;
        bool has_outgoing = false;
        bool has_incoming = false;
        for (const auto & entry : policy.vendor_entries) {
            has_outgoing |=
                entry.group_type == VendorGroupType::OUTGOING || entry.group_type == VendorGroupType::EQUIVALENT;
            has_incoming |=
                entry.group_type == VendorGroupType::INCOMING || entry.group_type == VendorGroupType::EQUIVALENT;
        }
        if (!has_outgoing) {
            throw VendorChangePolicyConfigFileError(
                M_("Configuration file \"{}\" uses version \"1.0\" which does not support"
                   " 'incoming_vendors' without 'outgoing_vendors'"),
                path.native());
        }
        if (!has_incoming) {
            throw VendorChangePolicyConfigFileError(
                M_("Configuration file \"{}\" uses version \"1.0\" which does not support"
                   " 'outgoing_vendors' without 'incoming_vendors'"),
                path.native());
        }
    }

    vendor_policies_def.push_back(std::move(policy));

    // Clear cached vendor masks
    vendor_masks.clear();
}

}  // namespace libdnf5::solv
