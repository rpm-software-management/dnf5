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

#ifndef DNF5_COMMANDS_COPR_COPR_CONFIG_HPP
#define DNF5_COMMANDS_COPR_COPR_CONFIG_HPP

#include <libdnf5/base/base.hpp>
#include <libdnf5/conf/config_parser.hpp>

namespace dnf5 {

class CoprConfig : public libdnf5::ConfigParser {
private:
    libdnf5::Base & base;

    void load_copr_config_file(const std::string & filename);
    void load_all_configuration();

public:
    /// Vendor change policy for the 'enable' command, set via the --vendor-change option,
    /// the "vendor_change" key in the "[main]" section of the Copr configuration, or the
    /// equivalent interactive prompt. The numeric values match the interactive menu's
    /// option numbers (1-4).
    enum class VendorChangeChoice {
        /// --vendor-change=false (or any OptionBool false value); also the fallback used when
        /// "vendor_change" is not set in the Copr configuration.
        /// No vendor change is allowed in either direction.
        NONE = 1,

        /// --vendor-change=to-repo.
        /// Packages from the enabled repositories may replace an already installed package
        /// from a different vendor.
        TO_COPR_REPO = 2,

        /// --vendor-change=from-repo.
        /// A package installed from the enabled repositories may be replaced by a package
        /// from a different vendor.
        FROM_COPR_REPO = 3,

        /// --vendor-change=true (or any OptionBool true value).
        /// Vendor change is allowed in both directions.
        BOTH = 4,
    };

    /// Parses a --vendor-change / "vendor_change" config value into a VendorChangeChoice.
    /// @param choice_str One of "to-repo", "from-repo", or any OptionBool-recognized boolean value.
    /// @throws libdnf5::OptionInvalidValueError if choice_str is none of the above.
    static VendorChangeChoice vendor_change_choice_from_string(const std::string & choice_str);

    /// Reads the "vendor_change" key from the "[main]" section of the Copr configuration.
    /// @return The configured choice, or VendorChangeChoice::NONE if "vendor_change" is not set.
    /// @throws std::runtime_error if "vendor_change" is set to an unrecognized value.
    VendorChangeChoice get_vendor_change_choice() const;

    explicit CoprConfig(libdnf5::Base & base);
    std::string get_hub_hostname(const std::string & hubspec);
    std::string get_hub_url(const std::string & hubspec);
    std::string get_repo_url(
        const std::string & hubspec,
        const std::string & ownername,
        const std::string & dirname,
        const std::string & name_version);
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_COPR_COPR_CONFIG_HPP
