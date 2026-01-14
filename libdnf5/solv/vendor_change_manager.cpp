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
constexpr std::array<std::string_view, 2> CONF_FILE_SUPPORTED_VERSIONS = {"1.0", "1.1"};

const std::map<std::string_view, sack::QueryCmp> VALID_COMPARATORS = {
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
};


#ifdef TOML11_COMPAT
inline auto location_first_line_num(const toml::source_location & location) {
    return location.line();
}
#else   // #ifdef TOML11_COMPAT
inline auto location_first_line_num(const toml::source_location & location) {
    return location.first_line_number();
}
#endif  // #ifdef TOML11_COMPAT


class VendorChangePolicyConfigFileError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::solv"; }
    const char * get_name() const noexcept override { return "VendorChangePolicyConfigFileError"; }
};

}  // namespace


VendorChangeManager::VendorChangeManager(const Pool & pool) : pool{pool} {}


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
        if (vendor_class_def.outgoing_vendors.empty()) {
            // Default to permit-all if no specific outgoing vendors are defined
            masks.outgoing_mask.add_grow(static_cast<int>(class_idx), EXTRA_CAPACITY);
        } else {
            for (const auto & vendor_def : vendor_class_def.outgoing_vendors) {
                if (sack::match_string(vendor_str, vendor_def.comparator, vendor_def.vendor)) {
                    if (!vendor_def.is_exclusion) {
                        masks.outgoing_mask.add_grow(static_cast<int>(class_idx), EXTRA_CAPACITY);
                    }
                    break;
                }
            }
        }
        if (vendor_class_def.incoming_vendors.empty()) {
            // Default to permit-all if no specific incoming vendors are defined
            masks.incoming_mask.add_grow(static_cast<int>(class_idx), EXTRA_CAPACITY);
        } else {
            for (const auto & vendor_def : vendor_class_def.incoming_vendors) {
                if (sack::match_string(vendor_str, vendor_def.comparator, vendor_def.vendor)) {
                    if (!vendor_def.is_exclusion) {
                        masks.incoming_mask.add_grow(static_cast<int>(class_idx), EXTRA_CAPACITY);
                    }
                    break;
                }
            }
        }
    }

    return vendor_masks.emplace_back(masks);
}


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
                   " \"equivalent_vendors\" with \"outgoing_vendors\" and \"incoming_vendors\""),
                path.native());
        }

        for (const auto & [element, value] : config.as_table()) {
            if (element == "version") {
                continue;
            }

            if (element != "outgoing_vendors" && element != "incoming_vendors" && element != "equivalent_vendors") {
                const auto location = value.location();
                throw VendorChangePolicyConfigFileError(
                    M_("Unknown key \"{}\" in file \"{}\" on line {}"),
                    element,
                    path.native(),
                    location_first_line_num(location));
            }

            enum class VendorsGroupType { OUTGOING, INCOMING, EQUIVALENT };
            const VendorsGroupType vendors_group_type =
                element == "outgoing_vendors"
                    ? VendorsGroupType::OUTGOING
                    : (element == "incoming_vendors" ? VendorsGroupType::INCOMING : VendorsGroupType::EQUIVALENT);

            for (const auto & vendor_entry : value.as_array()) {
                VendorChangePolicy::VendorDef vendor_def;
                vendor_def.comparator = sack::QueryCmp::EXACT;
                vendor_def.is_exclusion = false;

                bool is_vendor_set = false;
                for (const auto & [key, value] : vendor_entry.as_table()) {
                    if (key == "vendor") {
                        vendor_def.vendor = value.as_string();
                        is_vendor_set = true;
                    } else if (key == "comparator") {
                        auto it = VALID_COMPARATORS.find(value.as_string());
                        if (it == VALID_COMPARATORS.end()) {
                            const auto location = value.location();
                            throw VendorChangePolicyConfigFileError(
                                M_("Unknown comparator \"{}\" in file \"{}\" on line {}"),
                                value.as_string(),
                                path.native(),
                                location_first_line_num(location));
                        }
                        vendor_def.comparator = it->second;
                    } else if (key == "exclude") {
                        vendor_def.is_exclusion = value.as_boolean();
                    } else {
                        const auto location = value.location();
                        throw VendorChangePolicyConfigFileError(
                            M_("Unknown key \"{}\" in file \"{}\" on line {}"),
                            key,
                            path.native(),
                            location_first_line_num(location));
                    }
                }

                if (!is_vendor_set) {
                    const auto location = vendor_entry.location();
                    throw VendorChangePolicyConfigFileError(
                        M_("Missing \"vendor\" key in file \"{}\" for table entry on line {}"),
                        path.native(),
                        location_first_line_num(location));
                }

                switch (vendors_group_type) {
                    case VendorsGroupType::OUTGOING:
                        policy.outgoing_vendors.push_back(std::move(vendor_def));
                        break;
                    case VendorsGroupType::INCOMING:
                        policy.incoming_vendors.push_back(std::move(vendor_def));
                        break;
                    case VendorsGroupType::EQUIVALENT:
                        policy.outgoing_vendors.push_back(vendor_def);
                        policy.incoming_vendors.push_back(std::move(vendor_def));
                        break;
                }
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

    if (policy.outgoing_vendors.empty() && policy.incoming_vendors.empty()) {
        // Both lists are empty, so there is nothing to add
        return;
    }

    if (is_config_version_1_0 && policy.outgoing_vendors.empty()) {
        throw VendorChangePolicyConfigFileError(
            M_("Configuration file \"{}\" uses version \"1.0\" which does not support"
               " \"incoming_vendors\" without \"outgoing_vendors\""),
            path.native());
    }

    if (is_config_version_1_0 && policy.incoming_vendors.empty()) {
        throw VendorChangePolicyConfigFileError(
            M_("Configuration file \"{}\" uses version \"1.0\" which does not support"
               " \"outgoing_vendors\" without \"incoming_vendors\""),
            path.native());
    }

    vendor_policies_def.push_back(std::move(policy));

    // Clear cached vendor masks
    vendor_masks.clear();
}

}  // namespace libdnf5::solv
