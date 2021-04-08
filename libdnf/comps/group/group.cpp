#include <libdnf/comps/group/package.hpp>
#include <libdnf/comps/group/group.hpp>
#include <libdnf/comps/group/query.hpp>
#include <libdnf/comps/group/sack.hpp>
#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/comps_impl.hpp>
#include <libdnf/utils/xml.hpp>

extern "C" {
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solvable.h>
#include <solv/dataiterator.h>
}

#include <string>
#include <iostream>
#include <libxml/tree.h>


namespace libdnf::comps {


Group::~Group() {}


Group::Group(GroupQuery * query) : query(query->get_weak_ptr()) {}


void add_solvable_id(Group & group, Id solvable_id) {
    group.add_group_id(GroupId(solvable_id));
}


void add_solvable_ids(Group & group, std::vector<Id> solvable_ids) {
    for (Id solvable_id : solvable_ids) {
        group.add_group_id(GroupId(solvable_id));
    }
}


Group & Group::operator+=(const Group & rhs) {
    this->group_ids.insert(this->group_ids.begin(), rhs.group_ids.begin(), rhs.group_ids.end());
    return *this;
}


std::string lookup_str(Pool * pool, std::vector<GroupId> group_ids, Id key) {
    for (GroupId group_id: group_ids) {
        if (pool_lookup_str(pool, group_id.id, key)) {
            return pool_lookup_str(pool, group_id.id, key);
        }
    }
    return "";
}


std::string Group::get_groupid() const {
    std::string solvable_name(
        lookup_str(query->sack->comps.p_impl->get_pool(), group_ids, SOLVABLE_NAME));
    if (solvable_name.find(":") == std::string::npos) {
        return "";
    }
    return solvable_name.substr(solvable_name.find(":") + 1);
}


std::string Group::get_name() const {
    return lookup_str(query->sack->comps.p_impl->get_pool(), group_ids, SOLVABLE_SUMMARY);
}


std::string Group::get_description() const {
    return lookup_str(query->sack->comps.p_impl->get_pool(), group_ids, SOLVABLE_DESCRIPTION);
}


std::string Group::get_translated_name(const char * lang) const {
    std::string translation;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (GroupId group_id: group_ids) {
        Solvable * solvable = pool->solvables + group_id.id;
        if (solvable_lookup_str_lang(solvable, SOLVABLE_SUMMARY, lang, 1)) {
            translation = solvable_lookup_str_lang(solvable, SOLVABLE_SUMMARY, lang, 1);
            // Return translation only if it's different from the untranslated string.
            if (translation != solvable_lookup_str(solvable, SOLVABLE_SUMMARY)) {
                return translation;
            }
        }
    }
    return this->get_name();
}


// TODO(pkratoch): Test this
std::string Group::get_translated_name() const {
    std::string translation;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (GroupId group_id: group_ids) {
        Solvable * solvable = pool->solvables + group_id.id;
        if (solvable_lookup_str_poollang(solvable, SOLVABLE_SUMMARY)) {
            translation = solvable_lookup_str_poollang(solvable, SOLVABLE_SUMMARY);
            // Return translation only if it's different from the untranslated string.
            if (translation != solvable_lookup_str(solvable, SOLVABLE_SUMMARY)) {
                return translation;
            }
        }
    }
    return this->get_name();
}


std::string Group::get_translated_description(const char * lang) const {
    std::string translation;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (GroupId group_id: group_ids) {
        Solvable * solvable = pool->solvables + group_id.id;
        if (solvable_lookup_str_lang(solvable, SOLVABLE_DESCRIPTION, lang, 1)) {
            translation = solvable_lookup_str_lang(solvable, SOLVABLE_DESCRIPTION, lang, 1);
            // Return translation only if it's different from the untranslated string.
            if (translation != solvable_lookup_str(solvable, SOLVABLE_DESCRIPTION)) {
                return translation;
            }
        }
    }
    return this->get_description();
}


std::string Group::get_translated_description() const {
    std::string translation;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (GroupId group_id: group_ids) {
        Solvable * solvable = pool->solvables + group_id.id;
        if (solvable_lookup_str_poollang(solvable, SOLVABLE_DESCRIPTION)) {
            translation = solvable_lookup_str_poollang(solvable, SOLVABLE_DESCRIPTION);
            // Return translation only if it's different from the untranslated string.
            if (translation != solvable_lookup_str(solvable, SOLVABLE_DESCRIPTION)) {
                return translation;
            }
        }
    }
    return this->get_description();
}


std::string Group::get_order() const {
    return lookup_str(query->sack->comps.p_impl->get_pool(), group_ids, SOLVABLE_ORDER);
}


std::string Group::get_langonly() const {
    return lookup_str(query->sack->comps.p_impl->get_pool(), group_ids, SOLVABLE_LANGONLY);
}


bool Group::get_uservisible() const {
    Pool * pool = query->sack->comps.p_impl->get_pool();
    return pool_lookup_void(pool, group_ids[0].id, SOLVABLE_ISVISIBLE);
}


bool Group::get_default() const {
    Pool * pool = query->sack->comps.p_impl->get_pool();
    return pool_lookup_void(pool, group_ids[0].id, SOLVABLE_ISDEFAULT);
}


std::vector<Package> Group::get_packages() {
    // Return packages if they are already loaded
    if (!packages.empty()) {
        return packages;
    }

    Pool * pool = query->sack->comps.p_impl->get_pool();
    // Use only the first (highest priority) solvable for package lists
    Solvable * solvable = pool_id2solvable(pool, group_ids[0].id);

    // Load MANDATORY pacakges from solvable->requires
    if (solvable->requires) {
        for (Id * r_id = solvable->repo->idarraydata + solvable->requires; *r_id; ++r_id) {
            packages.push_back(Package(pool_id2str(pool, *r_id), PackageType::MANDATORY, ""));
        }
    }
    // Load DEFAULT and CONDITIONAL pacakges from solvable->recommends
    if (solvable->recommends) {
        for (Id * r_id = solvable->repo->idarraydata + solvable->recommends; *r_id; ++r_id) {
            if (strcmp(pool_id2rel(pool, *r_id), "") == 0) {
                packages.push_back(Package(pool_id2str(pool, *r_id), PackageType::DEFAULT, ""));
            }
            else {
                packages.push_back(Package(pool_id2str(pool, *r_id), PackageType::CONDITIONAL, pool_id2evr(pool, *r_id)));
            }
        }
    }
    // Load OPTIONAL pacakges from solvable->suggests
    if (solvable->suggests) {
        for (Id * r_id = solvable->repo->idarraydata + solvable->suggests; *r_id; ++r_id) {
            if (strcmp(pool_id2rel(pool, *r_id), "") == 0) {
                packages.push_back(Package(pool_id2str(pool, *r_id), PackageType::OPTIONAL, ""));
            }
        }
    }
    return packages;
}


std::vector<Package> Group::get_packages_of_type(PackageType type) {
    std::vector<Package> packages_ot_type;
    for (auto package : get_packages()) {
        if (package.get_type() == type) {
            packages_ot_type.push_back(package);
        }
    }
    return packages_ot_type;
}


std::set<std::string> Group::get_repos() const {
    std::set<std::string> result;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (GroupId group_id: group_ids) {
        Solvable * solvable = pool_id2solvable(pool, group_id.id);
        result.emplace(solvable->repo->name);
    }
    return result;
}


bool Group::get_installed() const {
    auto repos = get_repos();
    return repos.find("@System") != repos.end();
}


void Group::dump(const std::string & path) {
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
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (auto group_id : group_ids) {
        Dataiterator di;
        dataiterator_init(&di, pool, 0, group_id.id, 0, 0, 0);
        // Iterate over all data in the group solvable
        while (dataiterator_step(&di) != 0) {
            // If the content is NULL, skip
            if (!di.kv.str) {
                continue;
            }
            keyname = pool_id2str(pool, di.key->name);
            // If keyname starts with "solvable:summary:", it's a name translation
            if (keyname.rfind(summary_prefix, 0) == 0) {
                lang = keyname.substr(summary_prefix.length());
                // Add the lang into the set
                // If it's succesful (wasn't already present), create an XML node for this translation
                if(name_langs.insert(lang).second) {
                    node = utils::xml::add_subnode_with_text(node_group, "name", std::string(di.kv.str));
                    xmlNewProp(node, BAD_CAST "xml:lang", BAD_CAST lang.c_str());
                }
            }
            // If keyname starts with "solvable:description:", it's a description translation
            else if (keyname.rfind(description_prefix, 0) == 0) {
                lang = keyname.substr(description_prefix.length());
                // Add the lang into the set
                // If it's succesful (wasn't already present), create an XML node for this translation
                if(description_langs.insert(lang).second) {
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
    for (auto pkg: get_packages()) {
        // Create an XML node for this package
        node = utils::xml::add_subnode_with_text(node_packagelist, "packagereq", pkg.get_name());
        xmlNewProp(node, BAD_CAST "type", BAD_CAST pkg.get_type_string().c_str());
        if (pkg.get_type() == PackageType::CONDITIONAL) {
            xmlNewProp(node, BAD_CAST "requires", BAD_CAST pkg.get_condition().c_str());
        }
    }

    // Save the document
    if (xmlSaveFile(path.c_str(), doc) == -1) {
        throw std::runtime_error("failed to save xml document for comps");
    }

    // Memory free
    xmlFreeDoc(doc);
}

}  // namespace libdnf::comps
