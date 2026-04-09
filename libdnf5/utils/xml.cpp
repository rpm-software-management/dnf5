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


#include "xml.hpp"

#include <libxml/tree.h>

#include <algorithm>
#include <string>


namespace libdnf5::utils::xml {


__attribute__((__format__(printf, 2, 0))) void error_to_strings(void * ctx, const char * fmt, ...) {
    auto xml_errors = static_cast<std::vector<std::string> *>(ctx);
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    va_end(args);
    xml_errors->push_back(buffer);
}


std::vector<std::string> make_errors_unique(std::vector<std::string> xml_errors) {
    std::sort(xml_errors.begin(), xml_errors.end());
    auto it = std::unique(xml_errors.begin(), xml_errors.end());
    xml_errors.resize(static_cast<size_t>(std::distance(xml_errors.begin(), it)));
    return xml_errors;
}


xmlNodePtr new_node(const std::string & node_name) {
    xmlNodePtr node = xmlNewNode(NULL, BAD_CAST node_name.c_str());
    if (!node) {
        throw std::bad_alloc();
    }
    return node;
}


xmlAttrPtr new_prop(xmlNodePtr node, const std::string & name, const std::string & value) {
    xmlAttrPtr prop = xmlNewProp(node, BAD_CAST name.c_str(), BAD_CAST value.c_str());
    if (!prop) {
        throw std::bad_alloc();
    }
    return prop;
}


xmlNodePtr add_child(xmlNodePtr parent, xmlNodePtr child) {
    libdnf_assert(parent && child && parent != child, "Invalid parent or child node");
    if (!xmlAddChild(parent, child)) {
        throw std::bad_alloc();
    }
    return child;
}


xmlNodePtr add_subnode_with_text(xmlNodePtr parent, const std::string & child_name, const std::string & child_text) {
    xmlNodePtr node = new_node(child_name);
    add_child(parent, node);
    xmlNodePtr text = xmlNewText(BAD_CAST child_text.c_str());
    if (!text) {
        throw std::bad_alloc();
    }
    add_child(node, text);
    return node;
}


}  // namespace libdnf5::utils::xml
