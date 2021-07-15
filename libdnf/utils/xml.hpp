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
