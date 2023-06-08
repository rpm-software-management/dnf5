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

#include "libdnf/comps/group/group.hpp"

#include "solv/pool.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/string.hpp"
#include "utils/xml.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/comps/group/package.hpp"

extern "C" {
#include <solv/dataiterator.h>
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/solvable.h>
}

#include <libxml/tree.h>

#include <set>
#include <string>
#include <vector>


namespace libdnf5::comps {


Group::Group(const BaseWeakPtr & base) : base(base) {}


Group::Group(libdnf::Base & base) : base(base.get_weak_ptr()) {}


Group & Group::operator+=(const Group & rhs) {
    this->group_ids.insert(this->group_ids.begin(), rhs.group_ids.begin(), rhs.group_ids.end());
    return *this;
}


std::string Group::get_groupid() const {
    return solv::CompsPool::split_solvable_name(
               get_comps_pool(base).lookup_first_id_str<GroupId>(group_ids, SOLVABLE_NAME))
        .second;
}


std::string Group::get_name() const {
    return get_comps_pool(base).lookup_first_id_str<GroupId>(group_ids, SOLVABLE_SUMMARY);
}


std::string Group::get_description() const {
    return get_comps_pool(base).lookup_first_id_str<GroupId>(group_ids, SOLVABLE_DESCRIPTION);
}


std::string Group::get_translated_name(const char * lang) const {
    return get_comps_pool(base).get_translated_str<GroupId>(group_ids, SOLVABLE_SUMMARY, lang);
}


// TODO(pkratoch): Test this
std::string Group::get_translated_name() const {
    return get_comps_pool(base).get_translated_str<GroupId>(group_ids, SOLVABLE_SUMMARY);
}


std::string Group::get_translated_description(const char * lang) const {
    return get_comps_pool(base).get_translated_str<GroupId>(group_ids, SOLVABLE_DESCRIPTION, lang);
}


std::string Group::get_translated_description() const {
    return get_comps_pool(base).get_translated_str<GroupId>(group_ids, SOLVABLE_DESCRIPTION);
}


std::string Group::get_order() const {
    return get_comps_pool(base).lookup_first_id_str<GroupId>(group_ids, SOLVABLE_ORDER);
}


std::string Group::get_langonly() const {
    return get_comps_pool(base).lookup_first_id_str<GroupId>(group_ids, SOLVABLE_LANGONLY);
}


bool Group::get_uservisible() const {
    return get_comps_pool(base).lookup_void(group_ids[0].id, SOLVABLE_ISVISIBLE);
}


bool Group::get_default() const {
    return get_comps_pool(base).lookup_void(group_ids[0].id, SOLVABLE_ISDEFAULT);
}


std::vector<Package> Group::get_packages() {
    // Return packages if they are already loaded
    if (!packages.empty()) {
        return packages;
    }

    libdnf::solv::CompsPool & pool = get_comps_pool(base);

    // Use only the first (highest priority) solvable for package lists
    Solvable * solvable = pool.id2solvable(group_ids[0].id);

    // Load MANDATORY pacakges from solvable->requires
    if (solvable->dep_requires) {
        for (Id * r_id = solvable->repo->idarraydata + solvable->dep_requires; *r_id; ++r_id) {
            packages.push_back(Package(pool.id2str(*r_id), PackageType::MANDATORY, ""));
        }
    }
    // Load DEFAULT and CONDITIONAL pacakges from solvable->recommends
    if (solvable->dep_recommends) {
        for (Id * r_id = solvable->repo->idarraydata + solvable->dep_recommends; *r_id; ++r_id) {
            if (strcmp(pool.id2rel(*r_id), "") == 0) {
                packages.push_back(Package(pool.id2str(*r_id), PackageType::DEFAULT, ""));
            } else {
                packages.push_back(Package(pool.id2str(*r_id), PackageType::CONDITIONAL, pool.id2evr(*r_id)));
            }
        }
    }
    // Load OPTIONAL pacakges from solvable->suggests
    if (solvable->dep_suggests) {
        for (Id * r_id = solvable->repo->idarraydata + solvable->dep_suggests; *r_id; ++r_id) {
            if (strcmp(pool.id2rel(*r_id), "") == 0) {
                packages.push_back(Package(pool.id2str(*r_id), PackageType::OPTIONAL, ""));
            }
        }
    }
    return packages;
}


std::vector<Package> Group::get_packages_of_type(PackageType type) {
    std::vector<Package> packages_of_type;
    for (auto package : get_packages()) {
        if (static_cast<bool>(package.get_type() & type)) {
            packages_of_type.push_back(package);
        }
    }
    return packages_of_type;
}


std::set<std::string> Group::get_repos() const {
    std::set<std::string> result;
    for (GroupId group_id : group_ids) {
        Solvable * solvable = get_comps_pool(base).id2solvable(group_id.id);
        result.emplace(solvable->repo->name);
    }
    return result;
}


bool Group::get_installed() const {
    auto repos = get_repos();
    return repos.find("@System") != repos.end();
}


// libxml2 error handler. By default libxml2 prints errors directly to stderr which
// makes a mess of the outputs.
// This stores the errors in a vector of strings;
__attribute__((__format__(printf, 2, 0))) static void error_to_strings(void * ctx, const char * fmt, ...) {
    auto xml_errors = static_cast<std::vector<std::string> *>(ctx);
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    va_end(args);
    xml_errors->push_back(buffer);
}

void Group::serialize(const std::string & path) {
    std::vector<std::string> xml_errors;
    xmlSetGenericErrorFunc(&xml_errors, &error_to_strings);

    // Create doc with root node "comps"
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr node_comps = xmlNewNode(NULL, BAD_CAST "comps");
    xmlDocSetRootElement(doc, node_comps);

    // Create "group" node
    xmlNodePtr node_group = xmlNewNode(NULL, BAD_CAST "group");
    xmlAddChild(node_comps, node_group);

    // Add id, name, description, default, uservisible, display_order, langonly
    utils::xml::add_subnode_with_text(node_group, "id", get_groupid());
    utils::xml::add_subnode_with_text(node_group, "name", get_name());
    utils::xml::add_subnode_with_text(node_group, "description", get_description());
    utils::xml::add_subnode_with_text(node_group, "default", get_default() ? "true" : "false");
    utils::xml::add_subnode_with_text(node_group, "uservisible", get_uservisible() ? "true" : "false");
    utils::xml::add_subnode_with_text(node_group, "display_order", get_order());
    utils::xml::add_subnode_with_text(node_group, "langonly", get_langonly());

    // Add translations
    std::set<std::string> name_langs;
    std::set<std::string> description_langs;
    std::string summary_prefix = "solvable:summary:";
    std::string description_prefix = "solvable:description:";
    std::string keyname;
    std::string lang;
    xmlNodePtr node;

    libdnf5::solv::CompsPool & pool = get_comps_pool(base);

    for (auto group_id : group_ids) {
        Dataiterator di;
        dataiterator_init(&di, *pool, 0, group_id.id, 0, 0, 0);
        // Iterate over all data in the group solvable
        while (dataiterator_step(&di) != 0) {
            // If the content is NULL, skip
            if (!di.kv.str) {
                continue;
            }
            keyname = pool.id2str(di.key->name);
            // If keyname starts with "solvable:summary:", it's a name translation
            if (keyname.rfind(summary_prefix, 0) == 0) {
                lang = keyname.substr(summary_prefix.length());
                // Add the lang into the set
                // If it's succesful (wasn't already present), create an XML node for this translation
                if (name_langs.insert(lang).second) {
                    node = utils::xml::add_subnode_with_text(node_group, "name", std::string(di.kv.str));
                    xmlNewProp(node, BAD_CAST "xml:lang", BAD_CAST lang.c_str());
                }
            }
            // If keyname starts with "solvable:description:", it's a description translation
            else if (keyname.rfind(description_prefix, 0) == 0) {
                lang = keyname.substr(description_prefix.length());
                // Add the lang into the set
                // If it's succesful (wasn't already present), create an XML node for this translation
                if (description_langs.insert(lang).second) {
                    node = utils::xml::add_subnode_with_text(node_group, "description", std::string(di.kv.str));
                    xmlNewProp(node, BAD_CAST "xml:lang", BAD_CAST lang.c_str());
                }
            }
        }
        dataiterator_free(&di);
    }

    // Add packagelist
    xmlNodePtr node_packagelist = xmlNewNode(NULL, BAD_CAST "packagelist");
    xmlAddChild(node_group, node_packagelist);
    for (const auto & pkg : get_packages()) {
        // Create an XML node for this package
        node = utils::xml::add_subnode_with_text(node_packagelist, "packagereq", pkg.get_name());
        xmlNewProp(node, BAD_CAST "type", BAD_CAST pkg.get_type_string().c_str());
        if (pkg.get_type() == PackageType::CONDITIONAL) {
            xmlNewProp(node, BAD_CAST "requires", BAD_CAST pkg.get_condition().c_str());
        }
    }

    // Save the document
    if (xmlSaveFormatFileEnc(path.c_str(), doc, "utf-8", 1) == -1) {
        // There can be duplicit messages in the libxml2 errors so make them unique
        auto it = unique(xml_errors.begin(), xml_errors.end());
        xml_errors.resize(static_cast<size_t>(distance(xml_errors.begin(), it)));
        throw utils::xml::XMLSaveError(
            M_("Failed to save xml document for group \"{}\" to file \"{}\": {}"),
            get_groupid(),
            path,
            libdnf::utils::string::join(xml_errors, ", "));
    }

    // Memory free
    xmlFreeDoc(doc);
    // reset the error handler to default
    xmlSetGenericErrorFunc(NULL, NULL);
}

}  // namespace libdnf5::comps
