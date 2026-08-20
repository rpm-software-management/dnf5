// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "vendor_change_manager.hpp"

#include "pool.hpp"
#include "utils/fs/utils.hpp"
#include "utils/string.hpp"

#include "libdnf5/base/vendor_change_manager_errors.hpp"
#include "libdnf5/common/sack/match_string.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/patterns.hpp"

#include <toml.hpp>

#include <array>
#include <cctype>
#include <optional>
#include <utility>

namespace libdnf5::solv {

namespace {

bool is_string_comparator(sack::QueryCmp cmp) noexcept {
    auto base = cmp - sack::QueryCmp::NOT - sack::QueryCmp::ICASE;
    return base == sack::QueryCmp::EXACT || base == sack::QueryCmp::CONTAINS || base == sack::QueryCmp::STARTSWITH ||
           base == sack::QueryCmp::ENDSWITH || base == sack::QueryCmp::REGEX || base == sack::QueryCmp::GLOB;
}


bool is_relational_comparator(sack::QueryCmp cmp) noexcept {
    return cmp == sack::QueryCmp::EXACT || cmp == sack::QueryCmp::NOT_EXACT || cmp == sack::QueryCmp::GT ||
           cmp == sack::QueryCmp::GTE || cmp == sack::QueryCmp::LT || cmp == sack::QueryCmp::LTE;
}


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


bool is_relational_package_filter_func(
    VendorChangeManager::VendorChangePolicy::PackageDef::FilterFunction func) noexcept {
    return func == filter_package_evr || func == filter_package_epoch || func == filter_package_version ||
           func == filter_package_release;
}


VendorChangeManager::VendorChangePolicy::PackageDef::FilterFunction resolve_package_filter_func(
    const std::string & name) noexcept {
    if (name == "name") {
        return filter_package_name;
    }
    if (name == "source_name") {
        return filter_package_source_name;
    }
    if (name == "evr") {
        return filter_package_evr;
    }
    if (name == "epoch") {
        return filter_package_epoch;
    }
    if (name == "version") {
        return filter_package_version;
    }
    if (name == "release") {
        return filter_package_release;
    }
    if (name == "arch") {
        return filter_package_arch;
    }
    if (name == "repoid") {
        return filter_package_repoid;
    }
    if (name == "cmdline_repo") {
        return filter_package_cmdline_repo;
    }
    return nullptr;
}


std::string_view package_filter_func_to_name(VendorChangeManager::VendorChangePolicy::PackageDef::FilterFunction func) {
    if (func == filter_package_name) {
        return "name";
    }
    if (func == filter_package_source_name) {
        return "source_name";
    }
    if (func == filter_package_evr) {
        return "evr";
    }
    if (func == filter_package_epoch) {
        return "epoch";
    }
    if (func == filter_package_version) {
        return "version";
    }
    if (func == filter_package_release) {
        return "release";
    }
    if (func == filter_package_arch) {
        return "arch";
    }
    if (func == filter_package_repoid) {
        return "repoid";
    }
    if (func == filter_package_cmdline_repo) {
        return "cmdline_repo";
    }
    libdnf_throw_assertion("Unknown package filter function");
}


class VendorChangePolicyTomlFormatError : public base::VendorChangeManagerError {
public:
    using VendorChangeManagerError::VendorChangeManagerError;
    const char * get_domain_name() const noexcept override { return "libdnf5::solv"; }
    const char * get_name() const noexcept override { return "VendorChangePolicyTomlFormatError"; }
};


class VendorChangePolicyTomlFormat {
public:
    explicit VendorChangePolicyTomlFormat(std::filesystem::path path) : path{std::move(path)} {}

    VendorChangeManager::VendorChangePolicy parse() const;

    static std::string to_string(const VendorChangeManager::VendorChangePolicy & entries);

private:
    using VendorGroupType = VendorChangeManager::VendorChangePolicy::VendorGroupType;

    // supported config file version
    static constexpr std::array<std::string_view, 3> CONF_FILE_SUPPORTED_VERSIONS = {"1.0", "1.1", "1.2"};

    static inline const std::map<std::string_view, sack::QueryCmp> COMPARATORS = {
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

    static inline const std::map<std::string_view, sack::QueryCmp> COMPARATORS_1_2 = {
        {"NOT_STARTSWITH", sack::QueryCmp::NOT | sack::QueryCmp::STARTSWITH},
        {"NOT_ISTARTSWITH", sack::QueryCmp::NOT | sack::QueryCmp::ISTARTSWITH},
        {"NOT_ENDSWITH", sack::QueryCmp::NOT | sack::QueryCmp::ENDSWITH},
        {"NOT_IENDSWITH", sack::QueryCmp::NOT | sack::QueryCmp::IENDSWITH},
        {"NOT_REGEX", sack::QueryCmp::NOT | sack::QueryCmp::REGEX},
        {"NOT_IREGEX", sack::QueryCmp::NOT | sack::QueryCmp::IREGEX},
    };

    sack::QueryCmp string_to_comparator(
        const std::string & str_comparator, const std::string & cfg_version, std::size_t line_num) const;

    static std::string comparator_to_string(sack::QueryCmp comparator);

#ifdef TOML11_COMPAT
    static auto location_first_line_num(const toml::source_location & location) { return location.line(); }
#else   // #ifdef TOML11_COMPAT
    static auto location_first_line_num(const toml::source_location & location) { return location.first_line_number(); }
#endif  // #ifdef TOML11_COMPAT

    VendorChangeManager::VendorChangePolicy::PackageDef::Filter read_package_def_filter(
        const toml::value & filter_table, const std::string & cfg_version) const;

    std::filesystem::path path;
};


VendorChangeManager::VendorChangePolicy VendorChangePolicyTomlFormat::parse() const {
    VendorChangeManager::VendorChangePolicy policy;

    bool is_config_version_1_0;

    try {
        // Parse the TOML file
        auto config = toml::parse<toml::ordered_type_config>(path);

        // Check config file version
        const auto version = toml::find<std::optional<std::string>>(config, "version");
        if (!version) {
            throw VendorChangePolicyTomlFormatError(M_("Missing \"version\" key in file \"{}\""), path.native());
        }

        bool is_supported_version{false};
        for (const auto supported_version : CONF_FILE_SUPPORTED_VERSIONS) {
            if (version == supported_version) {
                is_supported_version = true;
                break;
            }
        }
        if (!is_supported_version) {
            throw VendorChangePolicyTomlFormatError(
                M_("Unsupported version \"{}\" in file \"{}\". Supported versions: {}"),
                *version,
                path.native(),
                libdnf5::utils::string::join(CONF_FILE_SUPPORTED_VERSIONS, ", "));
        }

        is_config_version_1_0 = version == "1.0";

        if (is_config_version_1_0 && config.contains("equivalent_vendors") &&
            (config.contains("outgoing_vendors") || config.contains("incoming_vendors"))) {
            throw VendorChangePolicyTomlFormatError(
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
                throw VendorChangePolicyTomlFormatError(
                    M_("Unknown key '{}' in file \"{}\" on line {}"),
                    element,
                    path.native(),
                    location_first_line_num(location));
            }

            if (group_type == GroupType::OUTGOING_PACKAGES || group_type == GroupType::INCOMING_PACKAGES) {
                if (is_config_version_1_0) {
                    const auto location = value.location();
                    throw VendorChangePolicyTomlFormatError(
                        M_("Configuration file \"{}\" uses version \"1.0\""
                           " which does not support key '{}' on line {}"),
                        path.native(),
                        element,
                        location_first_line_num(location));
                }

                for (const auto & entry : value.as_array()) {
                    VendorChangeManager::VendorChangePolicy::PackageDef package_def;
                    package_def.is_exclusion = false;

                    for (const auto & [key, value] : entry.as_table()) {
                        if (key == "filters") {
                            for (const auto & filter_entry : value.as_array()) {
                                auto filter = read_package_def_filter(filter_entry, *version);
                                if (filter.filter_func == filter_package_cmdline_repo &&
                                    group_type == GroupType::OUTGOING_PACKAGES) {
                                    const auto location = filter_entry.location();
                                    throw VendorChangePolicyTomlFormatError(
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
                            throw VendorChangePolicyTomlFormatError(
                                M_("Unknown key '{}' in file \"{}\" on line {}"),
                                key,
                                path.native(),
                                location_first_line_num(location));
                        }
                    }

                    if (package_def.filters.empty()) {
                        const auto location = entry.location();
                        throw VendorChangePolicyTomlFormatError(
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
                VendorChangeManager::VendorChangePolicy::VendorDef vendor_def;
                vendor_def.comparator = sack::QueryCmp::EXACT;
                vendor_def.is_exclusion = false;

                bool is_vendor_set = false;
                for (const auto & [key, value] : entry.as_table()) {
                    if (key == "vendor") {
                        vendor_def.vendor = value.as_string();
                        is_vendor_set = true;
                    } else if (key == "comparator") {
                        const auto line_num = location_first_line_num(value.location());
                        const auto comparator = string_to_comparator(value.as_string(), *version, line_num);
                        if (!is_string_comparator(comparator)) {
                            const auto location = value.location();
                            throw VendorChangePolicyTomlFormatError(
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
                        throw VendorChangePolicyTomlFormatError(
                            M_("Unknown key '{}' in file \"{}\" on line {}"),
                            key,
                            path.native(),
                            location_first_line_num(location));
                    }
                }

                if (!is_vendor_set) {
                    const auto location = entry.location();
                    throw VendorChangePolicyTomlFormatError(
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
                        throw VendorChangePolicyTomlFormatError(
                            M_("Invalid regex vendor pattern \"{}\" in file \"{}\" in table starting on line {}: {}"),
                            vendor_def.vendor,
                            path.native(),
                            location_first_line_num(location),
                            std::string(ex.what()));
                    }
                }

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
        throw VendorChangePolicyTomlFormatError(
            M_("Bad value type in file \"{}\" on line {}: {}"),
            path.native(),
            location_first_line_num(loc),
            std::string(ex.what()));
    } catch (const toml::exception & ex) {
        throw VendorChangePolicyTomlFormatError(
            M_("An error occurred when parsing file \"{}\": {}"), path.native(), std::string(ex.what()));
    }

    if (is_config_version_1_0 && !policy.vendor_entries.empty()) {
        bool has_outgoing = false;
        bool has_incoming = false;
        for (const auto & entry : policy.vendor_entries) {
            has_outgoing |=
                entry.group_type == VendorGroupType::OUTGOING || entry.group_type == VendorGroupType::EQUIVALENT;
            has_incoming |=
                entry.group_type == VendorGroupType::INCOMING || entry.group_type == VendorGroupType::EQUIVALENT;
        }
        if (!has_outgoing) {
            throw VendorChangePolicyTomlFormatError(
                M_("Configuration file \"{}\" uses version \"1.0\" which does not support"
                   " 'incoming_vendors' without 'outgoing_vendors'"),
                path.native());
        }
        if (!has_incoming) {
            throw VendorChangePolicyTomlFormatError(
                M_("Configuration file \"{}\" uses version \"1.0\" which does not support"
                   " 'outgoing_vendors' without 'incoming_vendors'"),
                path.native());
        }
    }

    return policy;
}


std::string VendorChangePolicyTomlFormat::to_string(const VendorChangeManager::VendorChangePolicy & policy) {
    std::string result = "version = '1.2'\n";

    // Vendor entries
    for (const auto & vendor_entry : policy.vendor_entries) {
        const char * section;
        switch (vendor_entry.group_type) {
            case VendorGroupType::OUTGOING:
                section = "outgoing_vendors";
                break;
            case VendorGroupType::INCOMING:
                section = "incoming_vendors";
                break;
            case VendorGroupType::EQUIVALENT:
                section = "equivalent_vendors";
                break;
            default:
                libdnf_throw_assertion("Invalid vendor change policy direction");
        }
        result += "\n[[";
        result += section;
        result += "]]\nvendor = ";
        result += toml::format(toml::value(vendor_entry.def.vendor));
        result += "\n";

        if (vendor_entry.def.comparator != sack::QueryCmp::EXACT) {
            result += "comparator = '";
            result += comparator_to_string(vendor_entry.def.comparator);
            result += "'\n";
        }
        if (vendor_entry.def.is_exclusion) {
            result += "exclude = true\n";
        }
    }

    // Package filter entries
    for (const auto & [pkg_entries, section] :
         {std::pair{policy.incoming_packages, "incoming_packages"},
          std::pair{policy.outgoing_packages, "outgoing_packages"}}) {
        for (const auto & pkg_entry : pkg_entries) {
            result += "\n[[";
            result += section;
            result += "]]\n";
            if (pkg_entry.is_exclusion) {
                result += "exclude = true\n";
            }
            result += "filters = [";
            bool first = true;
            for (const auto & filter : pkg_entry.filters) {
                if (!first) {
                    result += ',';
                }
                first = false;
                result += "\n  { filter = '";
                result += package_filter_func_to_name(filter.filter_func);
                result += "', value = ";
                result += toml::format(toml::value(filter.value));
                if (filter.comparator != sack::QueryCmp::EXACT) {
                    result += ", comparator = '";
                    result += comparator_to_string(filter.comparator);
                    result += "'";
                }
                result += " }";
            }
            result += "\n]\n";
        }
    }

    return result;
}


sack::QueryCmp VendorChangePolicyTomlFormat::string_to_comparator(
    const std::string & str_comparator, const std::string & cfg_version, std::size_t line_num) const {
    if (const auto it = COMPARATORS.find(str_comparator); it != COMPARATORS.end()) {
        return it->second;
    }
    const auto it_12 = COMPARATORS_1_2.find(str_comparator);
    if (it_12 == COMPARATORS_1_2.end()) {
        throw VendorChangePolicyTomlFormatError(
            M_("Unknown 'comparator' \"{}\" in file \"{}\" on line {}"), str_comparator, path.native(), line_num);
    }
    if (cfg_version != "1.2") {
        throw VendorChangePolicyTomlFormatError(
            M_("Configuration file \"{}\" uses version \"{}\" which does not support 'comparator' \"{}\""),
            path.native(),
            cfg_version,
            str_comparator);
    }
    return it_12->second;
}


std::string VendorChangePolicyTomlFormat::comparator_to_string(sack::QueryCmp comparator) {
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
    libdnf_throw_assertion("Invalid comparator");
}


VendorChangeManager::VendorChangePolicy::PackageDef::Filter VendorChangePolicyTomlFormat::read_package_def_filter(
    const toml::value & filter_table, const std::string & cfg_version) const {
    VendorChangeManager::VendorChangePolicy::PackageDef::Filter filter;
    filter.comparator = sack::QueryCmp::EXACT;

    bool filter_found = false;
    bool value_found = false;
    std::string filter_str;
    for (const auto & [key, value] : filter_table.as_table()) {
        if (key == "filter") {
            filter_str = value.as_string();
            filter.filter_func = resolve_package_filter_func(filter_str);
            if (!filter.filter_func) {
                const auto location = value.location();
                throw VendorChangePolicyTomlFormatError(
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
            filter.comparator = string_to_comparator(value.as_string(), cfg_version, line_num);
        } else {
            const auto location = value.location();
            throw VendorChangePolicyTomlFormatError(
                M_("Unknown key '{}' in file \"{}\" on line {}"),
                key,
                path.native(),
                location_first_line_num(location));
        }
    }
    if (!filter_found) {
        const auto location = filter_table.location();
        throw VendorChangePolicyTomlFormatError(
            M_("Missing 'filter' key in file \"{}\" for table entry on line {}"),
            path.native(),
            location_first_line_num(location));
    }
    if (!value_found) {
        const auto location = filter_table.location();
        throw VendorChangePolicyTomlFormatError(
            M_("Missing 'value' key in file \"{}\" for table entry on line {}"),
            path.native(),
            location_first_line_num(location));
    }

    if (filter.filter_func == filter_package_cmdline_repo) {
        if (filter.comparator != sack::QueryCmp::EXACT) {
            const auto location = filter_table.location();
            throw VendorChangePolicyTomlFormatError(
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
            throw VendorChangePolicyTomlFormatError(
                M_("Invalid 'value' \"{}\" in file \"{}\" in table starting on line {}."
                   " Only \"true\", \"1\", \"false\", \"0\" are supported"),
                filter.value,
                path.native(),
                location_first_line_num(location));
        }
    } else if (is_relational_package_filter_func(filter.filter_func)) {
        if (!is_relational_comparator(filter.comparator)) {
            const auto location = filter_table.location();
            throw VendorChangePolicyTomlFormatError(
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
            throw VendorChangePolicyTomlFormatError(
                M_("Filter \"{}\" in file \"{}\" in table starting on line {} "
                   "does not support comparator \"{}\""),
                filter_str,
                path.native(),
                location_first_line_num(location),
                comparator_to_string(filter.comparator));
        }
    }

    // Validate epoch value
    if (filter.filter_func == filter_package_epoch) {
        try {
            std::stoul(filter.value);
        } catch (const std::exception & ex) {
            const auto location = filter_table.location();
            throw VendorChangePolicyTomlFormatError(
                M_("Invalid epoch value \"{}\" in file \"{}\" in table starting on line {}"),
                filter.value,
                path.native(),
                location_first_line_num(location));
        }
    }

    // Validate regex pattern
    if (const auto cmp = filter.comparator - sack::QueryCmp::NOT - sack::QueryCmp::ICASE;
        cmp == sack::QueryCmp::REGEX) {
        try {
            sack::match_string("", filter.comparator, filter.value);
        } catch (const std::exception & ex) {
            const auto location = filter_table.location();
            throw VendorChangePolicyTomlFormatError(
                M_("Invalid regex \"{}\" in file \"{}\" in table starting on line {}: {}"),
                filter.value,
                path.native(),
                location_first_line_num(location),
                std::string(ex.what()));
        }
    }

    return filter;
}


class VendorChangePolicyCompactFormatError : public base::VendorChangeManagerError {
public:
    using VendorChangeManagerError::VendorChangeManagerError;
    const char * get_domain_name() const noexcept override { return "libdnf5::solv"; }
    const char * get_name() const noexcept override { return "VendorChangePolicyCompactFormatError"; }
};


class VendorChangePolicyCompactFormat {
public:
    explicit VendorChangePolicyCompactFormat(std::string_view input, std::string source)
        : input{input},
          source{std::move(source)} {}

    VendorChangeManager::VendorChangePolicy parse();

    static std::string to_string(const VendorChangeManager::VendorChangePolicy & policy);

private:
    using VendorGroupType = VendorChangeManager::VendorChangePolicy::VendorGroupType;
    using FilterFunction = VendorChangeManager::VendorChangePolicy::PackageDef::FilterFunction;

    void skip_white_spaces() noexcept {
        while (pos < input.size() &&
               (input[pos] == ' ' || input[pos] == '\t' || input[pos] == '\n' || input[pos] == '\r')) {
            ++pos;
        }
    }

    bool at_end() const noexcept { return pos >= input.size(); }

    char peek() const noexcept {
        if (at_end()) {
            return '\0';
        }
        return input[pos];
    }

    char advance() {
        if (at_end()) {
            throw_error(Error(M_("Unexpected end of input")));
        }
        return input[pos++];
    }

    bool match(char c) noexcept {
        if (peek() == c) {
            ++pos;
            return true;
        }
        return false;
    }

    void expect(char c) {
        skip_white_spaces();
        if (!match(c)) {
            if (at_end()) {
                throw_error(Error(M_("Expected '{}' but found end of input"), c));
            } else {
                throw_error(Error(M_("Expected '{}' but found '{}'"), c, peek()));
            }
        }
    }

    [[noreturn]] void throw_error(const Error & err) const {
        try {
            throw err;
        } catch (...) {
            libdnf5::throw_with_nested(VendorChangePolicyCompactFormatError(
                M_("Compact vendor change policy parse error in \"{}\" at position {}"), source, pos));
        }
    }

    std::string parse_quoted_string();

    static std::string to_quoted_string(const std::string & value);

    VendorGroupType parse_direction();

    sack::QueryCmp parse_comparator();

    static std::string comparator_to_string(sack::QueryCmp cmp);

    void parse_vendor_entry(VendorChangeManager::VendorChangePolicy & policy);

    FilterFunction resolve_filter_func(const std::string & field_name);

    std::string parse_field_name();

    VendorChangeManager::VendorChangePolicy::PackageDef::Filter parse_filter(VendorGroupType dir);

    void parse_package_entry(VendorChangeManager::VendorChangePolicy & policy);

    std::string_view input;
    std::string source;
    size_t pos = 0;
};


VendorChangeManager::VendorChangePolicy VendorChangePolicyCompactFormat::parse() {
    VendorChangeManager::VendorChangePolicy policy;
    pos = 0;

    skip_white_spaces();
    if (at_end()) {
        throw_error(Error(M_("Empty vendor policy string")));
    }

    if (peek() != '@') {
        parse_vendor_entry(policy);
        skip_white_spaces();
        while (peek() == ',') {
            ++pos;
            skip_white_spaces();
            parse_vendor_entry(policy);
            skip_white_spaces();
        }
    }

    if (match('@')) {
        skip_white_spaces();
        if (at_end()) {
            throw_error(Error(M_("Expected package filter after '@'")));
        }
        parse_package_entry(policy);
        skip_white_spaces();
        while (peek() == ',') {
            ++pos;
            skip_white_spaces();
            parse_package_entry(policy);
            skip_white_spaces();
        }
    }

    if (!at_end()) {
        throw_error(Error(M_("Unexpected character '{}'"), peek()));
    }

    if (policy.vendor_entries.empty() && policy.outgoing_packages.empty() && policy.incoming_packages.empty()) {
        throw_error(Error(M_("Empty policy: no vendor definitions or package filters specified")));
    }

    return policy;
}


std::string VendorChangePolicyCompactFormat::to_string(const VendorChangeManager::VendorChangePolicy & policy) {
    std::string result;

    // Vendor entries
    bool first = true;
    for (const auto & entry : policy.vendor_entries) {
        if (!first) {
            result += ',';
        }
        first = false;
        switch (entry.group_type) {
            case VendorChangeManager::VendorChangePolicy::VendorGroupType::OUTGOING:
                result += "out:";
                break;
            case VendorChangeManager::VendorChangePolicy::VendorGroupType::INCOMING:
                result += "in:";
                break;
            case VendorChangeManager::VendorChangePolicy::VendorGroupType::EQUIVALENT:
                result += "eq:";
                break;
            default:
                libdnf_throw_assertion("Invalid vendor change policy direction");
        }
        if (entry.def.is_exclusion) {
            result += 'e';
        }
        if (entry.def.comparator != sack::QueryCmp::EXACT) {
            result += comparator_to_string(entry.def.comparator);
        }
        result += to_quoted_string(entry.def.vendor);
    }

    // Package filter entries
    if (!policy.outgoing_packages.empty() || !policy.incoming_packages.empty()) {
        result += '@';
        first = true;
        for (const auto & [pkg_entries, prefix] :
             {std::pair{policy.incoming_packages, "in:"}, std::pair{policy.outgoing_packages, "out:"}}) {
            for (const auto & pkg_entry : pkg_entries) {
                if (!first) {
                    result += ',';
                }
                first = false;
                result += prefix;
                if (pkg_entry.is_exclusion) {
                    result += 'e';
                }
                result += '[';
                bool first_filter = true;
                for (const auto & filter : pkg_entry.filters) {
                    if (!first_filter) {
                        result += ',';
                    }
                    first_filter = false;
                    result += package_filter_func_to_name(filter.filter_func);
                    result += comparator_to_string(filter.comparator);
                    result += to_quoted_string(filter.value);
                }
                result += ']';
            }
        }
    }

    return result;
}


std::string VendorChangePolicyCompactFormat::parse_quoted_string() {
    if (peek() != '"') {
        throw_error(Error(M_("Expected quoted string starting with '\"'")));
    }
    ++pos;
    std::string result;
    while (!at_end() && peek() != '"') {
        if (peek() == '\\') {
            ++pos;
            if (at_end()) {
                throw_error(Error(M_("Unterminated escape sequence")));
            }
            char c = input[pos++];
            if (c == '"' || c == '\\') {
                result += c;
            } else {
                throw_error(Error(M_("Invalid escape sequence '\\{}'. Only '\\\"' and '\\\\' are supported"), c));
            }
        } else {
            result += input[pos++];
        }
    }
    if (at_end()) {
        throw_error(Error(M_("Unterminated quoted string")));
    }
    ++pos;
    return result;
}


std::string VendorChangePolicyCompactFormat::to_quoted_string(const std::string & value) {
    std::string quoted_escaped;
    quoted_escaped.reserve(value.size() + 2);
    quoted_escaped += '"';
    for (char c : value) {
        if (c == '"' || c == '\\') {
            quoted_escaped += '\\';
        }
        quoted_escaped += c;
    }
    quoted_escaped += '"';
    return quoted_escaped;
};


VendorChangePolicyCompactFormat::VendorGroupType VendorChangePolicyCompactFormat::parse_direction() {
    if (pos + 2 < input.size() && input.substr(pos, 3) == "out") {
        pos += 3;
        return VendorGroupType::OUTGOING;
    }
    if (pos + 1 < input.size() && input.substr(pos, 2) == "eq") {
        pos += 2;
        return VendorGroupType::EQUIVALENT;
    }
    if (pos + 1 < input.size() && input.substr(pos, 2) == "in") {
        pos += 2;
        return VendorGroupType::INCOMING;
    }
    throw_error(Error(M_("Expected direction prefix 'in', 'out', or 'eq'")));
}


sack::QueryCmp VendorChangePolicyCompactFormat::parse_comparator() {
    const bool negated = peek() == '!';
    if (negated) {
        ++pos;
    }

    const bool case_insensitive = peek() == 'i';
    if (case_insensitive) {
        ++pos;
    }

    bool err = false;
    sack::QueryCmp base_cmp;
    if (peek() == '=' && pos + 1 < input.size() && input[pos + 1] == '~') {
        pos += 2;
        base_cmp = sack::QueryCmp::REGEX;
    } else if (peek() == '=' && pos + 1 < input.size() && input[pos + 1] == '*') {
        pos += 2;
        base_cmp = sack::QueryCmp::GLOB;
    } else if (peek() == '=') {
        ++pos;
        base_cmp = sack::QueryCmp::EXACT;
    } else if (peek() == '^') {
        ++pos;
        base_cmp = sack::QueryCmp::STARTSWITH;
    } else if (peek() == '$') {
        ++pos;
        base_cmp = sack::QueryCmp::ENDSWITH;
    } else if (peek() == '*') {
        ++pos;
        base_cmp = sack::QueryCmp::CONTAINS;
    } else if (peek() == '>' && pos + 1 < input.size() && input[pos + 1] == '=') {
        pos += 2;
        base_cmp = sack::QueryCmp::GTE;
    } else if (peek() == '>') {
        ++pos;
        base_cmp = sack::QueryCmp::GT;
    } else if (peek() == '<' && pos + 1 < input.size() && input[pos + 1] == '=') {
        pos += 2;
        base_cmp = sack::QueryCmp::LTE;
    } else if (peek() == '<') {
        ++pos;
        base_cmp = sack::QueryCmp::LT;
    } else {
        err = true;
    }

    sack::QueryCmp result;
    if (!err) {
        result = base_cmp;
        if (negated) {
            result = result | sack::QueryCmp::NOT;
        }
        if (case_insensitive) {
            result = result | sack::QueryCmp::ICASE;
        }
    }

    if (err || (!is_string_comparator(result) && !is_relational_comparator(result))) {
        throw_error(Error(
            M_("Expected comparison operator: relational (<, <=, >, >=) or string (=, ^, $, *, =*, =~; optionally "
               "prefixed with ! and/or i")));
    }

    return result;
}


std::string VendorChangePolicyCompactFormat::comparator_to_string(sack::QueryCmp cmp) {
    libdnf_assert(is_string_comparator(cmp) || is_relational_comparator(cmp), "Invalid comparator");

    std::string result;

    if ((cmp & sack::QueryCmp::NOT) == sack::QueryCmp::NOT) {
        result += '!';
    }
    if ((cmp & sack::QueryCmp::ICASE) == sack::QueryCmp::ICASE) {
        result += 'i';
    }
    auto base = cmp - sack::QueryCmp::NOT - sack::QueryCmp::ICASE;
    if (base == sack::QueryCmp::EXACT) {
        result += '=';
    } else if (base == sack::QueryCmp::STARTSWITH) {
        result += '^';
    } else if (base == sack::QueryCmp::ENDSWITH) {
        result += '$';
    } else if (base == sack::QueryCmp::CONTAINS) {
        result += '*';
    } else if (base == sack::QueryCmp::GLOB) {
        result += "=*";
    } else if (base == sack::QueryCmp::REGEX) {
        result += "=~";
    } else if (base == sack::QueryCmp::GT) {
        result += '>';
    } else if (base == sack::QueryCmp::GTE) {
        result += ">=";
    } else if (base == sack::QueryCmp::LT) {
        result += '<';
    } else if (base == sack::QueryCmp::LTE) {
        result += "<=";
    }

    return result;
};


void VendorChangePolicyCompactFormat::parse_vendor_entry(VendorChangeManager::VendorChangePolicy & policy) {
    const auto dir = parse_direction();

    expect(':');

    skip_white_spaces();
    const bool is_exclusion = peek() == 'e';
    if (is_exclusion) {
        ++pos;
    }

    skip_white_spaces();
    const sack::QueryCmp comparator = peek() == '"' ? sack::QueryCmp::EXACT : parse_comparator();

    skip_white_spaces();
    std::string vendor = parse_quoted_string();

    if (!is_string_comparator(comparator)) {
        throw_error(
            Error(M_("Vendor definition does not support relational comparators. "
                     "Use string comparators: =, ^, $, *, =*, =~")));
    }

    if (const auto cmp = comparator - sack::QueryCmp::NOT - sack::QueryCmp::ICASE; cmp == sack::QueryCmp::REGEX) {
        try {
            sack::match_string("", comparator, vendor);
        } catch (const std::exception & ex) {
            throw_error(Error(M_("Invalid regex vendor pattern '{}': {}"), vendor, std::string(ex.what())));
        }
    }

    VendorChangeManager::VendorChangePolicy::VendorDef vendor_def;
    vendor_def.vendor = std::move(vendor);
    vendor_def.comparator = comparator;
    vendor_def.is_exclusion = is_exclusion;

    policy.vendor_entries.emplace_back(dir, std::move(vendor_def));
}


VendorChangePolicyCompactFormat::FilterFunction VendorChangePolicyCompactFormat::resolve_filter_func(
    const std::string & field_name) {
    if (auto result = resolve_package_filter_func(field_name)) {
        return result;
    }
    throw_error(Error(M_("Unknown filter field name '{}'"), field_name));
}


std::string VendorChangePolicyCompactFormat::parse_field_name() {
    size_t start = pos;
    while (!at_end() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        ++pos;
    }
    if (pos == start) {
        throw_error(Error(M_("Expected filter field name")));
    }
    return std::string(input.substr(start, pos - start));
}


VendorChangeManager::VendorChangePolicy::PackageDef::Filter VendorChangePolicyCompactFormat::parse_filter(
    VendorGroupType dir) {
    VendorChangeManager::VendorChangePolicy::PackageDef::Filter filter;

    std::string field_name = parse_field_name();
    filter.filter_func = resolve_filter_func(field_name);

    skip_white_spaces();
    filter.comparator = parse_comparator();

    skip_white_spaces();
    filter.value = parse_quoted_string();

    if (filter.filter_func == filter_package_epoch) {
        try {
            std::stoul(filter.value);
        } catch (...) {
            throw_error(Error(M_("Invalid epoch value \"{}\""), filter.value));
        }
    } else if (filter.filter_func == filter_package_cmdline_repo) {
        if (filter.comparator != sack::QueryCmp::EXACT) {
            throw_error(Error(M_("Filter 'cmdline_repo' only supports the '=' (EXACT) comparator")));
        }
        if (dir == VendorGroupType::OUTGOING) {
            throw_error(Error(M_("Filter 'cmdline_repo' is only allowed in incoming packages ('in:')")));
        }

        // Optimization: bool value is stored in a string (empty string == false).
        // This avoids expensive string comparisons during filtering
        if (filter.value == "1" || filter.value == "true") {
            std::string{"1"}.swap(filter.value);  // store non-empty string
        } else if (filter.value == "0" || filter.value == "false") {
            std::string{}.swap(filter.value);  // store empty string
        } else {
            throw_error(Error(
                M_("Invalid value '{}' for 'cmdline_repo' filter. Use 'true', '1', 'false', or '0'"), filter.value));
        }
    } else if (is_relational_package_filter_func(filter.filter_func)) {
        if (!is_relational_comparator(filter.comparator)) {
            throw_error(Error(M_("Filter '{}' only supports relational comparators: =, !=, >, >=, <, <="), field_name));
        }
    } else {
        if (!is_string_comparator(filter.comparator)) {
            throw_error(Error(M_("Filter '{}' only supports string comparators: =, ^, $, *, =*, =~"), field_name));
        }
    }

    if (const auto cmp = filter.comparator - sack::QueryCmp::NOT - sack::QueryCmp::ICASE;
        cmp == sack::QueryCmp::REGEX) {
        try {
            sack::match_string("", filter.comparator, filter.value);
        } catch (const std::exception & ex) {
            throw_error(Error(M_("Invalid regex '{}': {}"), filter.value, std::string(ex.what())));
        }
    }

    return filter;
}


void VendorChangePolicyCompactFormat::parse_package_entry(VendorChangeManager::VendorChangePolicy & policy) {
    const auto dir = parse_direction();
    if (dir == VendorGroupType::EQUIVALENT) {
        throw_error(Error(M_("Package filters do not support 'eq' direction. Use 'in' or 'out'")));
    }

    expect(':');

    skip_white_spaces();
    const bool is_exclusion = peek() == 'e';
    if (is_exclusion) {
        ++pos;
    }

    expect('[');

    VendorChangeManager::VendorChangePolicy::PackageDef package_def;
    package_def.is_exclusion = is_exclusion;

    skip_white_spaces();
    package_def.filters.push_back(parse_filter(dir));
    skip_white_spaces();
    while (peek() == ',') {
        ++pos;
        skip_white_spaces();
        package_def.filters.push_back(parse_filter(dir));
        skip_white_spaces();
    }

    expect(']');

    if (dir == VendorGroupType::INCOMING) {
        policy.incoming_packages.push_back(std::move(package_def));
    } else {
        policy.outgoing_packages.push_back(std::move(package_def));
    }
}

}  // namespace


VendorChangeManager::VendorChangeManager(const Pool & pool) : pool{pool} {}


void VendorChangeManager::add_policy_from_toml(const std::filesystem::path & path) {
    VendorChangePolicyTomlFormat parser{path};
    auto policy = parser.parse();

    if (policy.outgoing_packages.empty() && policy.incoming_packages.empty() && policy.vendor_entries.empty()) {
        // All lists are empty, so there is nothing to add
        return;
    }

    policy.source = (path.is_absolute() ? "file://" : "file:") + path.string();

    add_policy(std::move(policy));
}


void VendorChangeManager::add_policy_from_compact(std::string_view policy_str, std::string_view source) {
    VendorChangePolicyCompactFormat parser{policy_str, std::string(source)};
    auto policy = parser.parse();

    if (policy.outgoing_packages.empty() && policy.incoming_packages.empty() && policy.vendor_entries.empty()) {
        // All lists are empty, so there is nothing to add
        return;
    }

    policy.source = source;

    add_policy(std::move(policy));
}


void VendorChangeManager::clear_policies() {
    vendor_policies_def.clear();
    vendor_masks.clear();
}


std::size_t VendorChangeManager::remove_policies_matching_source(const std::string & source_pattern) {
    std::size_t removed_count = 0;

    auto it = vendor_policies_def.begin();
    if (utils::is_glob_pattern(source_pattern.c_str())) {
        // Remove policies whose source matches the pattern
        while (it != vendor_policies_def.end()) {
            if (sack::match_string(it->source, sack::QueryCmp::GLOB, source_pattern)) {
                it = vendor_policies_def.erase(it);
                ++removed_count;
            } else {
                ++it;
            }
        }
    } else {
        // Remove policies whose source is equal to the pattern
        while (it != vendor_policies_def.end()) {
            if (it->source == source_pattern) {
                it = vendor_policies_def.erase(it);
                ++removed_count;
            } else {
                ++it;
            }
        }
    }

    if (removed_count != 0) {
        // Clear cached vendor masks
        vendor_masks.clear();
    }

    return removed_count;
}


const std::string & VendorChangeManager::get_policy_source(std::size_t index) const {
    if (index >= vendor_policies_def.size()) {
        throw base::VendorChangeManagerError(
            M_("{func}(): Policy index {idx} is out of range"),
            NamedErrorArg("func", __func__),
            NamedErrorArg("idx", index));
    }
    return vendor_policies_def[index].source;
}


std::string VendorChangeManager::get_policy_as_toml(std::size_t index) const {
    if (index >= vendor_policies_def.size()) {
        throw base::VendorChangeManagerError(
            M_("{func}(): Policy index {idx} is out of range"),
            NamedErrorArg("func", __func__),
            NamedErrorArg("idx", index));
    }
    return VendorChangePolicyTomlFormat::to_string(vendor_policies_def[index]);
}


std::string VendorChangeManager::get_policy_as_compact(std::size_t index) const {
    if (index >= vendor_policies_def.size()) {
        throw base::VendorChangeManagerError(
            M_("{func}(): Policy index {idx} is out of range"),
            NamedErrorArg("func", __func__),
            NamedErrorArg("idx", index));
    }
    return VendorChangePolicyCompactFormat::to_string(vendor_policies_def[index]);
}


std::string VendorChangeManager::convert_policy_toml_to_compact(const std::filesystem::path & path) {
    VendorChangePolicyTomlFormat parser{path};
    auto policy = parser.parse();
    return VendorChangePolicyCompactFormat::to_string(policy);
}


std::string VendorChangeManager::convert_policy_compact_to_toml(std::string_view compact_str, std::string_view source) {
    VendorChangePolicyCompactFormat parser{compact_str, std::string(source)};
    auto policy = parser.parse();
    return VendorChangePolicyTomlFormat::to_string(policy);
}


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


void VendorChangeManager::add_policy(VendorChangePolicy && policy) {
    vendor_policies_def.push_back(std::move(policy));

    // Clear cached vendor masks
    vendor_masks.clear();
}


const VendorChangeManager::VendorChangeMasks & VendorChangeManager::get_vendor_change_masks(Id vendor) {
    constexpr int EXTRA_CAPACITY = 7;
    static const VendorChangeMasks empty_masks;

    if (vendor == 0 || vendor_policies_def.empty()) {
        return empty_masks;
    }

    // Check if masks for this vendor are already cached
    if (auto it = vendor_masks.find(vendor); it != vendor_masks.end()) {
        return it->second;
    }

    // Create new masks for this vendor
    VendorChangeMasks masks;
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

    // Insert into map and return reference to the inserted value
    return vendor_masks.emplace(vendor, std::move(masks)).first->second;
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

}  // namespace libdnf5::solv
