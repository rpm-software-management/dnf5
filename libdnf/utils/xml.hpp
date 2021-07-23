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

#ifndef LIBDNF_UTILS_XML_HPP
#define LIBDNF_UTILS_XML_HPP

#include <libxml/tree.h>


namespace libdnf::utils::xml {

xmlNodePtr add_subnode_with_text(xmlNodePtr parent, std::string child_name, std::string child_text) {
    xmlNodePtr node = xmlNewNode(NULL, BAD_CAST child_name.c_str());
    xmlAddChild(parent, node);
    xmlAddChild(node, xmlNewText(BAD_CAST child_text.c_str()));
    return node;
}

}  // namespace libdnf::utils::xml

#endif  // LIBDNF_UTILS_XML_HPP
