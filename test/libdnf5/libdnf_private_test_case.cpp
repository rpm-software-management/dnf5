// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#include "libdnf_private_test_case.hpp"

#include "../shared/private_accessor.hpp"
#include "base/base_impl.hpp"
#include "utils/string.hpp"

#include <libdnf5/rpm/nevra.hpp>


namespace {

// Accessor of private Base::p_impl, see private_accessor.hpp
create_private_getter_template;
create_getter(priv_impl, &libdnf5::Base::p_impl);
create_getter(add_rpm_package, &libdnf5::repo::Repo::add_rpm_package);

}  // namespace

libdnf5::rpm::Package LibdnfPrivateTestCase::add_system_pkg(
    const std::string & relative_path, libdnf5::transaction::TransactionItemReason reason) {
    // parse out the NA from the package path to set the reason for the installed package
    auto filename_toks = libdnf5::utils::string::split(relative_path, "/");
    auto basename_toks = libdnf5::utils::string::rsplit(filename_toks.back(), ".", 2);
    auto nevras = libdnf5::rpm::Nevra::parse(basename_toks.front());
    CPPUNIT_ASSERT_MESSAGE("Couldn't parse NEVRA from package path: \"" + relative_path + "\"", !nevras.empty());
    auto na = nevras[0].get_name() + "." + nevras[0].get_arch();

    (base.*get(priv_impl()))->get_system_state().set_package_reason(na, reason);

    return (*(repo_sack->get_system_repo()).*get(add_rpm_package{}))(
        PROJECT_BINARY_DIR "/test/data/" + relative_path, false);
}
