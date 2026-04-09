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

#ifndef LIBDNF5_UTILS_XML_HPP
#define LIBDNF5_UTILS_XML_HPP

#include "libdnf5/common/exception.hpp"

#include <libxml/tree.h>
#include <libxml/xmlerror.h>

#include <string>
#include <vector>


namespace libdnf5::utils::xml {

struct XMLSaveError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::utils"; }
    const char * get_name() const noexcept override { return "XMLSaveError"; }
};


/// Restore the default generic error handler when destroyed.
struct GenericErrorFuncGuard {
    GenericErrorFuncGuard(void * ctx, xmlGenericErrorFunc handler) noexcept { xmlSetGenericErrorFunc(ctx, handler); }
    ~GenericErrorFuncGuard() { xmlSetGenericErrorFunc(NULL, NULL); }

    GenericErrorFuncGuard(const GenericErrorFuncGuard &) = delete;
    GenericErrorFuncGuard & operator=(const GenericErrorFuncGuard &) = delete;
    GenericErrorFuncGuard(GenericErrorFuncGuard &&) = delete;
    GenericErrorFuncGuard & operator=(GenericErrorFuncGuard &&) = delete;
};


// libxml2 error handler. By default libxml2 prints errors directly to stderr which
// makes a mess of the outputs.
// This stores the errors in a vector of strings.
__attribute__((__format__(printf, 2, 0))) void error_to_strings(void * ctx, const char * fmt, ...);


// There can be duplicate messages in the libxml2 errors, so make them unique.
std::vector<std::string> make_errors_unique(std::vector<std::string> xml_errors);


struct XmlDocDeleter {
    void operator()(xmlDoc * doc) const noexcept {
        if (doc != nullptr) {
            xmlFreeDoc(doc);
        }
    }
};


xmlNodePtr new_node(const std::string & node_name);


xmlAttrPtr new_prop(xmlNodePtr node, const std::string & name, const std::string & value);


xmlNodePtr add_child(xmlNodePtr parent, xmlNodePtr child);


xmlNodePtr add_subnode_with_text(xmlNodePtr parent, const std::string & child_name, const std::string & child_text);

}  // namespace libdnf5::utils::xml

#endif  // LIBDNF5_UTILS_XML_HPP
