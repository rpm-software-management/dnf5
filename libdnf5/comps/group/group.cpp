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

#include "libdnf5/comps/group/group.hpp"

#include "solv/pool.hpp"
#include "utils/string.hpp"
#include "utils/xml.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/comps/group/package.hpp"
#include "libdnf5/comps/group/query.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

extern "C" {
#include <solv/dataiterator.h>
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/solvable.h>
}

#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <limits.h>

#include <set>
#include <string>
#include <vector>


namespace libdnf5::comps {

class Group::Impl {
public:
    explicit Impl(const libdnf5::BaseWeakPtr & base) : base(base) {}

private:
    friend Group;

    libdnf5::BaseWeakPtr base;

    // Corresponds to solvable ids for this group (libsolv doesn't merge groups, so there are multiple solvables
    // for one groupid).
    // The order is given by the repoids of the originating repositories. The repoids that come later in the alphabet
    // are preferred (e.g. the description is taken from solvable from repository "B", even though there is a different
    // description in repository "A"). A notable case are repositories "fedora" and "updates" where the "updates"
    // repository is preferred, and coincidentally, this is what we want, because "updates" contains more up-to-date
    // definitions.
    std::vector<GroupId> group_ids;

    std::vector<Package> packages;
};

Group::Group(const BaseWeakPtr & base) : p_impl(std::make_unique<Impl>(base)) {}

Group::Group(libdnf5::Base & base) : Group(base.get_weak_ptr()) {}

Group::~Group() = default;

Group::Group(const Group & src) : p_impl(new Impl(*src.p_impl)) {}
Group::Group(Group && src) noexcept = default;

Group & Group::operator=(const Group & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
Group & Group::operator=(Group && src) noexcept = default;


Group & Group::operator+=(const Group & rhs) {
    p_impl->group_ids.insert(p_impl->group_ids.begin(), rhs.p_impl->group_ids.begin(), rhs.p_impl->group_ids.end());
    return *this;
}

bool Group::operator==(const Group & rhs) const noexcept {
    return p_impl->group_ids == rhs.p_impl->group_ids && p_impl->base == rhs.p_impl->base;
}
bool Group::operator!=(const Group & rhs) const noexcept {
    return p_impl->group_ids != rhs.p_impl->group_ids || p_impl->base != rhs.p_impl->base;
}
// Compare Groups by groupid and then by repoids (the string ids, so that it's deterministic even when loaded in a different order).
bool Group::operator<(const Group & rhs) const {
    return get_groupid() < rhs.get_groupid() || get_repos() < rhs.get_repos();
}


libdnf5::BaseWeakPtr Group::get_base() {
    return p_impl->base;
}

std::string Group::get_groupid() const {
    return solv::CompsPool::split_solvable_name(
               get_comps_pool(p_impl->base).lookup_first_id_str<GroupId>(p_impl->group_ids, SOLVABLE_NAME))
        .second;
}


std::string Group::get_name() const {
    return get_comps_pool(p_impl->base).lookup_first_id_str<GroupId>(p_impl->group_ids, SOLVABLE_SUMMARY);
}


std::string Group::get_description() const {
    return get_comps_pool(p_impl->base).lookup_first_id_str<GroupId>(p_impl->group_ids, SOLVABLE_DESCRIPTION);
}


std::string Group::get_translated_name(const char * lang) const {
    return get_comps_pool(p_impl->base).get_translated_str<GroupId>(p_impl->group_ids, SOLVABLE_SUMMARY, lang);
}


// TODO(pkratoch): Test this
std::string Group::get_translated_name() const {
    return get_comps_pool(p_impl->base).get_translated_str<GroupId>(p_impl->group_ids, SOLVABLE_SUMMARY);
}


std::string Group::get_translated_description(const char * lang) const {
    return get_comps_pool(p_impl->base).get_translated_str<GroupId>(p_impl->group_ids, SOLVABLE_DESCRIPTION, lang);
}


std::string Group::get_translated_description() const {
    return get_comps_pool(p_impl->base).get_translated_str<GroupId>(p_impl->group_ids, SOLVABLE_DESCRIPTION);
}


std::string Group::get_order() const {
    return get_comps_pool(p_impl->base).lookup_first_id_str<GroupId>(p_impl->group_ids, SOLVABLE_ORDER);
}


int Group::get_order_int() const {
    try {
        return std::stoi(get_order());
    } catch (const std::invalid_argument &) {
        return INT_MAX;
    } catch (const std::out_of_range &) {
        return INT_MAX;
    }
}


std::string Group::get_langonly() const {
    return get_comps_pool(p_impl->base).lookup_first_id_str<GroupId>(p_impl->group_ids, SOLVABLE_LANGONLY);
}


bool Group::get_uservisible() const {
    return get_comps_pool(p_impl->base).lookup_void(p_impl->group_ids[0].id, SOLVABLE_ISVISIBLE);
}


bool Group::get_default() const {
    return get_comps_pool(p_impl->base).lookup_void(p_impl->group_ids[0].id, SOLVABLE_ISDEFAULT);
}


std::vector<Package> Group::get_packages() {
    // Return packages if they are already loaded
    if (!p_impl->packages.empty()) {
        return p_impl->packages;
    }

    libdnf5::solv::CompsPool & pool = get_comps_pool(p_impl->base);

    std::set<const char *> package_ids_mandatory;
    std::set<const char *> package_ids_default;
    std::set<std::pair<const char *, const char *>> package_ids_conditional;
    std::set<const char *> package_ids_optional;

    for (auto group_id : p_impl->group_ids) {
        Solvable * solvable = pool.id2solvable(group_id.id);

        // Load MANDATORY packages from solvable->requires
        if (solvable->dep_requires) {
            for (Id * r_id = solvable->repo->idarraydata + solvable->dep_requires; *r_id; ++r_id) {
                const auto package_name = pool.id2str(*r_id);
                if (package_ids_mandatory.emplace(package_name).second) {
                    p_impl->packages.emplace_back(package_name, PackageType::MANDATORY, "");
                }
            }
        }
        // Load DEFAULT and CONDITIONAL packages from solvable->recommends
        if (solvable->dep_recommends) {
            for (Id * r_id = solvable->repo->idarraydata + solvable->dep_recommends; *r_id; ++r_id) {
                const auto package_name = pool.id2str(*r_id);
                if (strcmp(pool.id2rel(*r_id), "") == 0) {
                    if (package_ids_default.emplace(package_name).second) {
                        p_impl->packages.emplace_back(package_name, PackageType::DEFAULT, "");
                    }
                } else {
                    const auto condition = pool.id2evr(*r_id);
                    if (package_ids_conditional.emplace(package_name, condition).second) {
                        p_impl->packages.emplace_back(package_name, PackageType::CONDITIONAL, condition);
                    }
                }
            }
        }
        // Load OPTIONAL packages from solvable->suggests
        if (solvable->dep_suggests) {
            for (Id * r_id = solvable->repo->idarraydata + solvable->dep_suggests; *r_id; ++r_id) {
                const auto package_name = pool.id2str(*r_id);
                if (strcmp(pool.id2rel(*r_id), "") == 0) {
                    if (package_ids_optional.emplace(package_name).second) {
                        p_impl->packages.emplace_back(package_name, PackageType::OPTIONAL, "");
                    }
                }
            }
        }
    }
    return p_impl->packages;
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
    for (GroupId group_id : p_impl->group_ids) {
        Solvable * solvable = get_comps_pool(p_impl->base).id2solvable(group_id.id);
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

    libdnf5::solv::CompsPool & pool = get_comps_pool(p_impl->base);

    for (auto group_id : p_impl->group_ids) {
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
                // If it's successful (wasn't already present), create an XML node for this translation
                if (name_langs.insert(lang).second) {
                    node = utils::xml::add_subnode_with_text(node_group, "name", std::string(di.kv.str));
                    xmlNewProp(node, BAD_CAST "xml:lang", BAD_CAST lang.c_str());
                }
            }
            // If keyname starts with "solvable:description:", it's a description translation
            else if (keyname.rfind(description_prefix, 0) == 0) {
                lang = keyname.substr(description_prefix.length());
                // Add the lang into the set
                // If it's successful (wasn't already present), create an XML node for this translation
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
    auto save_result = xmlSaveFormatFileEnc(path.c_str(), doc, "utf-8", 1);

    // Memory free
    xmlFreeDoc(doc);
    // reset the error handler to default
    xmlSetGenericErrorFunc(NULL, NULL);

    if (save_result == -1) {
        // There can be duplicit messages in the libxml2 errors so make them unique
        auto it = unique(xml_errors.begin(), xml_errors.end());
        xml_errors.resize(static_cast<size_t>(distance(xml_errors.begin(), it)));
        throw utils::xml::XMLSaveError(
            M_("Failed to save xml document for group \"{}\" to file \"{}\": {}"),
            get_groupid(),
            path,
            libdnf5::utils::string::join(xml_errors, ", "));
    }
}

libdnf5::transaction::TransactionItemReason Group::get_reason() const {
    comps::GroupQuery installed_query(p_impl->base);
    installed_query.filter_installed(true);
    installed_query.filter_groupid(get_groupid());
    if (!installed_query.empty()) {
        auto reason = p_impl->base->p_impl->get_system_state().get_group_reason(get_groupid());

        if (reason == libdnf5::transaction::TransactionItemReason::NONE) {
            return libdnf5::transaction::TransactionItemReason::EXTERNAL_USER;
        }

        return reason;
    }

    return libdnf5::transaction::TransactionItemReason::NONE;
}

void Group::add_group_id(const GroupId & group_id) {
    p_impl->group_ids.push_back(group_id);
}

}  // namespace libdnf5::comps
