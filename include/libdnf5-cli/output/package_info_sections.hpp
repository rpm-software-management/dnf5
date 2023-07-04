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


#ifndef LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP
#define LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP

#include "package_list_sections.hpp"
#include "utils/string.hpp"

#include "libdnf5-cli/utils/units.hpp"

#include <libdnf5/rpm/package_set.hpp>
#include <libsmartcols/libsmartcols.h>

#include <string>

namespace libdnf5::cli::output {

class PackageInfoSections : public PackageListSections {
public:
    PackageInfoSections() : PackageListSections() {}
    template <class Package>
    bool add_package(
        Package pkg,
        const std::string & heading = "",
        const std::unique_ptr<PkgColorizer> & colorizer = nullptr,
        const std::vector<Package> & obsoletes = {}) {
        struct libscols_line * first_line = add_line("Name", pkg.get_name());
        if (colorizer) {
            scols_line_set_color(first_line, colorizer->get_pkg_color(pkg).c_str());
        }

        add_line("Epoch", pkg.get_epoch());
        add_line("Version", pkg.get_version());
        add_line("Release", pkg.get_release());
        add_line("Architecture", pkg.get_arch());

        if (!obsoletes.empty()) {
            auto iterator = obsoletes.begin();
            add_line("Obsoletes", iterator->get_full_nevra());
            ++iterator;
            for (; iterator != obsoletes.end(); ++iterator) {
                add_line("", iterator->get_full_nevra());
            }
        }

        if (!pkg.is_installed()) {
            add_line("Download size", utils::units::format_size_aligned(static_cast<int64_t>(pkg.get_download_size())));
        }
        add_line("Installed size", utils::units::format_size_aligned(static_cast<int64_t>(pkg.get_install_size())));
        if (pkg.get_arch() != "src") {
            add_line("Source", pkg.get_sourcerpm());
        }
        if (pkg.is_installed()) {
            add_line("From repository", pkg.get_from_repo_id());
        } else {
            add_line("Repository", pkg.get_repo_id());
        }
        add_line("Summary", pkg.get_summary());
        add_line("URL", pkg.get_url());
        add_line("License", pkg.get_license());

        auto lines = libdnf5::utils::string::split(pkg.get_description(), "\n");
        auto iterator = lines.begin();
        add_line("Description", *iterator);
        ++iterator;
        for (; iterator != lines.end(); ++iterator) {
            add_line("", *iterator);
        }
        struct libscols_line * last_line = add_line("Vendor", pkg.get_vendor());

        // for info output keep each package as a separate section, which means
        // an empty line is printed between packages resulting in better readability
        sections.emplace_back(heading, first_line, last_line);
        return true;
    }
    bool virtual add_section(
        const std::string & heading,
        const libdnf5::rpm::PackageSet & pkg_set,
        const std::unique_ptr<PkgColorizer> & colorizer = nullptr,
        const std::map<libdnf5::rpm::PackageId, std::vector<libdnf5::rpm::Package>> & obsoletes = {}) override;

    void setup_cols() override;

private:
    struct libscols_line * add_line(const std::string & key, const std::string & value);
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP
