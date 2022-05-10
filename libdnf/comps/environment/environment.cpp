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

#include "libdnf/comps/environment/environment.hpp"

#include "comps/pool_utils.hpp"
#include "solv/pool.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/xml.hpp"

#include "libdnf/base/base.hpp"

extern "C" {
#include <solv/dataiterator.h>
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/solvable.h>
}

#include <libxml/tree.h>

#include <set>
#include <string>
#include <string_view>
#include <vector>


namespace libdnf::comps {


Environment::Environment(const libdnf::BaseWeakPtr & base) : base(base) {}


Environment::Environment(libdnf::Base & base) : base(base.get_weak_ptr()) {}


Environment & Environment::operator+=(const Environment & rhs) {
    this->environment_ids.insert(this->environment_ids.begin(), rhs.environment_ids.begin(), rhs.environment_ids.end());
    return *this;
}


std::string Environment::get_environmentid() const {
    return split_solvable_name(lookup_str<EnvironmentId>(get_pool(base), environment_ids, SOLVABLE_NAME)).second;
}


std::string Environment::get_name() const {
    return lookup_str<EnvironmentId>(get_pool(base), environment_ids, SOLVABLE_SUMMARY);
}


std::string Environment::get_description() const {
    return lookup_str<EnvironmentId>(get_pool(base), environment_ids, SOLVABLE_DESCRIPTION);
}


std::string Environment::get_translated_name(const char * lang) const {
    return get_translated_str<EnvironmentId>(get_pool(base), environment_ids, SOLVABLE_SUMMARY, lang);
}


// TODO(pkratoch): Test this
std::string Environment::get_translated_name() const {
    return get_translated_str<EnvironmentId>(get_pool(base), environment_ids, SOLVABLE_SUMMARY);
}


std::string Environment::get_translated_description(const char * lang) const {
    return get_translated_str<EnvironmentId>(get_pool(base), environment_ids, SOLVABLE_DESCRIPTION, lang);
}


std::string Environment::get_translated_description() const {
    return get_translated_str<EnvironmentId>(get_pool(base), environment_ids, SOLVABLE_DESCRIPTION);
}


std::string Environment::get_order() const {
    return lookup_str<EnvironmentId>(get_pool(base), environment_ids, SOLVABLE_ORDER);
}


std::vector<std::string> load_groups_from_pool(libdnf::solv::Pool & pool, Id environment_id, bool required = true) {
    Solvable * solvable = pool.id2solvable(environment_id);
    Offset offset;
    if (required) {
        offset = solvable->dep_requires;
    } else {
        offset = solvable->dep_suggests;
    }

    std::vector<std::string> groups;
    std::string_view group_solvable_name;

    if (offset) {
        for (Id * r_id = solvable->repo->idarraydata + offset; *r_id; ++r_id) {
            group_solvable_name = pool.id2str(*r_id);
            groups.push_back(split_solvable_name(group_solvable_name).second);
        }
    }

    return groups;
}


std::vector<std::string> Environment::get_groups() {
    if (groups.empty()) {
        groups = load_groups_from_pool(get_pool(base), environment_ids[0].id);
    }
    return groups;
}


std::vector<std::string> Environment::get_optional_groups() {
    if (optional_groups.empty()) {
        optional_groups = load_groups_from_pool(get_pool(base), environment_ids[0].id, false);
    }
    return optional_groups;
}


std::set<std::string> Environment::get_repos() const {
    std::set<std::string> result;
    for (EnvironmentId environment_id : environment_ids) {
        Solvable * solvable = get_pool(base).id2solvable(environment_id.id);
        result.emplace(solvable->repo->name);
    }
    return result;
}


bool Environment::get_installed() const {
    auto repos = get_repos();
    return repos.find("@System") != repos.end();
}


void Environment::dump(const std::string & path) {
    // Create doc with root node "comps"
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr node_comps = xmlNewNode(NULL, BAD_CAST "comps");
    xmlDocSetRootElement(doc, node_comps);

    // Create "environment" node
    xmlNodePtr node_environment = xmlNewNode(NULL, BAD_CAST "environment");
    xmlAddChild(node_comps, node_environment);

    // Add id, name, description, display_order
    utils::xml::add_subnode_with_text(node_environment, "id", get_environmentid());
    utils::xml::add_subnode_with_text(node_environment, "name", get_name());
    utils::xml::add_subnode_with_text(node_environment, "description", get_description());
    utils::xml::add_subnode_with_text(node_environment, "display_order", get_order());

    // Add translations
    std::set<std::string> name_langs;
    std::set<std::string> description_langs;
    std::string summary_prefix = "solvable:summary:";
    std::string description_prefix = "solvable:description:";
    std::string keyname;
    std::string lang;
    xmlNodePtr node;

    libdnf::solv::Pool & pool = get_pool(base);

    for (auto environment_id : environment_ids) {
        Dataiterator di;
        dataiterator_init(&di, *pool, 0, environment_id.id, 0, 0, 0);
        // Iterate over all data in the environment solvable
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
                    node = utils::xml::add_subnode_with_text(node_environment, "name", std::string(di.kv.str));
                    xmlNewProp(node, BAD_CAST "xml:lang", BAD_CAST lang.c_str());
                }
            }
            // If keyname starts with "solvable:description:", it's a description translation
            else if (keyname.rfind(description_prefix, 0) == 0) {
                lang = keyname.substr(description_prefix.length());
                // Add the lang into the set
                // If it's succesful (wasn't already present), create an XML node for this translation
                if (description_langs.insert(lang).second) {
                    node = utils::xml::add_subnode_with_text(node_environment, "description", std::string(di.kv.str));
                    xmlNewProp(node, BAD_CAST "xml:lang", BAD_CAST lang.c_str());
                }
            }
        }
        dataiterator_free(&di);
    }

    // Add grouplist
    xmlNodePtr node_grouplist = xmlNewNode(NULL, BAD_CAST "grouplist");
    xmlAddChild(node_environment, node_grouplist);
    for (const auto & group : get_groups()) {
        // Create an XML node for this package
        node = utils::xml::add_subnode_with_text(node_grouplist, "groupid", group);
    }
    xmlNodePtr node_optionlist = xmlNewNode(NULL, BAD_CAST "optionlist");
    xmlAddChild(node_environment, node_optionlist);
    for (const auto & group : get_optional_groups()) {
        // Create an XML node for this package
        node = utils::xml::add_subnode_with_text(node_optionlist, "groupid", group);
    }

    // Save the document
    if (xmlSaveFormatFileEnc(path.c_str(), doc, "utf-8", 1) == -1) {
        throw utils::xml::XMLSaveError(M_("failed to save xml document for comps"));
    }

    // Memory free
    xmlFreeDoc(doc);
}

}  // namespace libdnf::comps
