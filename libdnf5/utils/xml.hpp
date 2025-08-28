// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_UTILS_XML_HPP
#define LIBDNF5_UTILS_XML_HPP

#include "libdnf5/common/exception.hpp"

#include <libxml/tree.h>

#include <string>


namespace libdnf5::utils::xml {

struct XMLSaveError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::utils"; }
    const char * get_name() const noexcept override { return "XMLSaveError"; }
};


xmlNodePtr add_subnode_with_text(xmlNodePtr parent, std::string child_name, std::string child_text);

}  // namespace libdnf5::utils::xml

#endif  // LIBDNF5_UTILS_XML_HPP
