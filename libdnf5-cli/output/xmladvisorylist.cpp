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

#include "libdnf5-cli/output/xmladvisorylist.hpp"

#include "utils/string.hpp"

#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#include <libdnf5/utils/xml.hpp>

#include <unistd.h>
#include <iostream>

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

namespace libdnf5::cli::output {

namespace {
    void add_advisory_to_node(xmlNodePtr node, auto & advisory) {
        xmlNodePtr update_node = xmlNewChild(node, NULL, BAD_CAST "update", NULL);
        xmlNewProp(update_node, BAD_CAST "from", BAD_CAST advisory.get_vendor().c_str());
        xmlNewProp(update_node, BAD_CAST "status", BAD_CAST advisory.get_status().c_str());
        xmlNewProp(update_node, BAD_CAST "type", BAD_CAST advisory.get_type().c_str());
        xmlNewProp(update_node, BAD_CAST "version", BAD_CAST "2.0");

        utils::xml::add_subnode_with_text(update_node, "id", advisory.get_name());
        utils::xml::add_subnode_with_text(update_node, "severity", advisory.get_severity());
        utils::xml::add_subnode_with_text(update_node, "title", advisory.get_title());

        std::string message = advisory.get_message();
        if (message.size() > 0)
            utils::xml::add_subnode_with_text(update_node, "message", message);

        // libsolv uses the later of issued/updated, so just display as updated
        // This is probably a FIXME
        unsigned long long buildtime = advisory.get_buildtime();
        xmlNodePtr buildtime_node = xmlNewChild(update_node, NULL, BAD_CAST "updated", NULL);
        xmlNewProp(buildtime_node, BAD_CAST "date", BAD_CAST  libdnf5::utils::string::format_epoch(buildtime).c_str());

        utils::xml::add_subnode_with_text(update_node, "description", advisory.get_description());

        std::string rights = advisory.get_rights();
        if (rights.size() > 0)
            utils::xml::add_subnode_with_text(update_node, "rights", rights);

        auto references = advisory.get_references();
        if (references.size() > 0) {
            xmlNodePtr references_node = xmlNewChild(update_node, NULL, BAD_CAST "references", NULL);
            for (auto & ref : advisory.get_references()) {
                xmlNodePtr ref_node = xmlNewChild(references_node, NULL, BAD_CAST "reference", NULL);
                xmlNewProp(ref_node, BAD_CAST "href", BAD_CAST ref.get_url().c_str());
                xmlNewProp(ref_node, BAD_CAST "id", BAD_CAST ref.get_id().c_str());
                xmlNewProp(ref_node, BAD_CAST "title", BAD_CAST ref.get_title().c_str());
                xmlNewProp(ref_node, BAD_CAST "type", BAD_CAST ref.get_type().c_str());
            }
        }
        auto collections = advisory.get_collections();
        if (collections.size() > 0) {
            for (auto & collection : collections) {
                auto pkgs = collection.get_packages();
                if (pkgs.size() > 0) {
                    xmlNodePtr pkglist_node = xmlNewChild(update_node, NULL, BAD_CAST "pkglist", NULL);
                    xmlNodePtr collection_node = xmlNewChild(pkglist_node, NULL, BAD_CAST "collection", NULL);
                    for (auto & pkg : pkgs) {
                        xmlNodePtr package_node = xmlNewChild(collection_node, NULL, BAD_CAST "package", NULL);
                        xmlNewProp(package_node, BAD_CAST "name", BAD_CAST pkg.get_name().c_str());
                        xmlNewProp(package_node, BAD_CAST "version", BAD_CAST pkg.get_version().c_str());
                        xmlNewProp(package_node, BAD_CAST "release", BAD_CAST pkg.get_release().c_str());
                        xmlNewProp(package_node, BAD_CAST "epoch", BAD_CAST pkg.get_epoch().c_str());
                        xmlNewProp(package_node, BAD_CAST "arch", BAD_CAST pkg.get_arch().c_str());
                        std::string filename = pkg.get_filename();
                        if (filename.size() > 0)
                            xmlNewChild(package_node, NULL, BAD_CAST "filename", BAD_CAST filename.c_str());
                        if (pkg.get_reboot_suggested())
                            xmlNewChild(package_node, NULL, BAD_CAST "reboot_suggested", BAD_CAST "True");
                        if (pkg.get_restart_suggested())
                            xmlNewChild(package_node, NULL, BAD_CAST "restart_suggested", BAD_CAST "True");
                        if (pkg.get_relogin_suggested())
                            xmlNewChild(package_node, NULL, BAD_CAST "relogin_suggested", BAD_CAST "True");
                    }
                }
            }
        }
    }
}

void print_xmladvisorylist(
    libdnf5::advisory::AdvisoryQuery advisories) {
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr node_updates = xmlNewNode(NULL, BAD_CAST "updates");
    xmlDocSetRootElement(doc, node_updates);

    for (auto advisory : advisories) {
        add_advisory_to_node(node_updates, advisory);
    }

    xmlChar *mem;
    int sz;
    xmlDocDumpFormatMemory(doc, &mem, &sz, 1);
    std::cout << (char*)mem << std::endl;
    xmlFree(mem);

    xmlFreeDoc(doc);
}

}
