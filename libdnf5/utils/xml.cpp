// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "xml.hpp"

#include <libxml/tree.h>

#include <string>


namespace libdnf5::utils::xml {


xmlNodePtr add_subnode_with_text(xmlNodePtr parent, std::string child_name, std::string child_text) {
    xmlNodePtr node = xmlNewNode(NULL, BAD_CAST child_name.c_str());
    xmlAddChild(parent, node);
    xmlAddChild(node, xmlNewText(BAD_CAST child_text.c_str()));
    return node;
}


}  // namespace libdnf5::utils::xml
