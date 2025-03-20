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

#include "libdnf5/comps/environment/environment.hpp"

#include "solv/pool.hpp"
#include "utils/xml.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/comps/environment/query.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

extern "C" {
#include <solv/dataiterator.h>
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/solvable.h>
}

#include <libxml/tree.h>
#include <limits.h>

#include <set>
#include <string>
#include <string_view>
#include <vector>


namespace libdnf5::comps {

class Environment::Impl {
public:
    explicit Impl(const libdnf5::BaseWeakPtr & base) : base(base) {}

private:
    friend Environment;

    libdnf5::BaseWeakPtr base;

    // Corresponds to solvable ids for this environment (libsolv doesn't merge groups, so there are multiple solvables
    // for one environmentid).
    // The order is given by the repoids of the originating repositories. The repoids that come later in the alphabet
    // are preferred (e.g. the description is taken from solvable from repository "B", even though there is a different
    // description in repository "A"). A notable case are repositories "fedora" and "updates" where the "updates"
    // repository is preferred, and coincidentally, this is what we want, because "updates" contains more up-to-date
    // definitions.
    std::vector<EnvironmentId> environment_ids;

    std::vector<std::string> groups;
    std::vector<std::string> optional_groups;
};


Environment::Environment(const libdnf5::BaseWeakPtr & base) : p_impl(std::make_unique<Impl>(base)) {}

Environment::Environment(libdnf5::Base & base) : Environment(base.get_weak_ptr()) {}

Environment::~Environment() = default;

Environment::Environment(const Environment & src) : p_impl(new Impl(*src.p_impl)) {}
Environment::Environment(Environment && src) noexcept = default;

Environment & Environment::operator=(const Environment & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
Environment & Environment::operator=(Environment && src) noexcept = default;

Environment & Environment::operator+=(const Environment & rhs) {
    p_impl->environment_ids.insert(
        p_impl->environment_ids.begin(), rhs.p_impl->environment_ids.begin(), rhs.p_impl->environment_ids.end());
    return *this;
}

bool Environment::operator==(const Environment & rhs) const noexcept {
    return p_impl->environment_ids == rhs.p_impl->environment_ids && p_impl->base == rhs.p_impl->base;
}

bool Environment::operator!=(const Environment & rhs) const noexcept {
    return p_impl->environment_ids != rhs.p_impl->environment_ids || p_impl->base != rhs.p_impl->base;
}

bool Environment::operator<(const Environment & rhs) const {
    return get_environmentid() < rhs.get_environmentid() || get_repos() < rhs.get_repos();
}

libdnf5::BaseWeakPtr Environment::get_base() {
    return p_impl->base;
}

std::string Environment::get_environmentid() const {
    return solv::CompsPool::split_solvable_name(
               get_comps_pool(p_impl->base).lookup_first_id_str<EnvironmentId>(p_impl->environment_ids, SOLVABLE_NAME))
        .second;
}


std::string Environment::get_name() const {
    return get_comps_pool(p_impl->base).lookup_first_id_str<EnvironmentId>(p_impl->environment_ids, SOLVABLE_SUMMARY);
}


std::string Environment::get_description() const {
    return get_comps_pool(p_impl->base)
        .lookup_first_id_str<EnvironmentId>(p_impl->environment_ids, SOLVABLE_DESCRIPTION);
}


std::string Environment::get_translated_name(const char * lang) const {
    return get_comps_pool(p_impl->base)
        .get_translated_str<EnvironmentId>(p_impl->environment_ids, SOLVABLE_SUMMARY, lang);
}


// TODO(pkratoch): Test this
std::string Environment::get_translated_name() const {
    return get_comps_pool(p_impl->base).get_translated_str<EnvironmentId>(p_impl->environment_ids, SOLVABLE_SUMMARY);
}


std::string Environment::get_translated_description(const char * lang) const {
    return get_comps_pool(p_impl->base)
        .get_translated_str<EnvironmentId>(p_impl->environment_ids, SOLVABLE_DESCRIPTION, lang);
}


std::string Environment::get_translated_description() const {
    return get_comps_pool(p_impl->base)
        .get_translated_str<EnvironmentId>(p_impl->environment_ids, SOLVABLE_DESCRIPTION);
}


std::string Environment::get_order() const {
    return get_comps_pool(p_impl->base).lookup_first_id_str<EnvironmentId>(p_impl->environment_ids, SOLVABLE_ORDER);
}


int Environment::get_order_int() const {
    try {
        return std::stoi(get_order());
    } catch (const std::invalid_argument &) {
        return INT_MAX;
    } catch (const std::out_of_range &) {
        return INT_MAX;
    }
}


std::vector<std::string> load_groups_from_pool(
    libdnf5::solv::CompsPool & pool, const std::vector<EnvironmentId> & environment_ids, bool required = true) {
    std::set<std::string> groups;
    std::string_view group_solvable_name;

    for (auto environment_id : environment_ids) {
        Solvable * solvable = pool.id2solvable(environment_id.id);
        Offset offset;
        if (required) {
            offset = solvable->dep_requires;
        } else {
            offset = solvable->dep_suggests;
        }

        if (offset) {
            for (Id * r_id = solvable->repo->idarraydata + offset; *r_id; ++r_id) {
                group_solvable_name = pool.id2str(*r_id);
                groups.emplace(solv::CompsPool::split_solvable_name(group_solvable_name).second);
            }
        }
    }

    return std::vector(groups.begin(), groups.end());
}


std::vector<std::string> Environment::get_groups() {
    if (p_impl->groups.empty()) {
        p_impl->groups = load_groups_from_pool(get_comps_pool(p_impl->base), p_impl->environment_ids);
    }
    return p_impl->groups;
}


std::vector<std::string> Environment::get_optional_groups() {
    if (p_impl->optional_groups.empty()) {
        p_impl->optional_groups = load_groups_from_pool(get_comps_pool(p_impl->base), p_impl->environment_ids, false);
    }
    return p_impl->optional_groups;
}

libdnf5::transaction::TransactionItemReason Environment::get_reason() const {
    comps::EnvironmentQuery installed_query(p_impl->base);
    installed_query.filter_installed(true);
    installed_query.filter_environmentid(get_environmentid());
    if (!installed_query.empty()) {
        auto reason = p_impl->base->p_impl->get_system_state().get_environment_reason(get_environmentid());

        if (reason == libdnf5::transaction::TransactionItemReason::NONE) {
            return libdnf5::transaction::TransactionItemReason::EXTERNAL_USER;
        }

        return reason;
    }

    return libdnf5::transaction::TransactionItemReason::NONE;
}


std::set<std::string> Environment::get_repos() const {
    std::set<std::string> result;
    for (EnvironmentId environment_id : p_impl->environment_ids) {
        Solvable * solvable = get_comps_pool(p_impl->base).id2solvable(environment_id.id);
        result.emplace(solvable->repo->name);
    }
    return result;
}


bool Environment::get_installed() const {
    auto repos = get_repos();
    return repos.find("@System") != repos.end();
}


void Environment::serialize(const std::string & path) {
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

    libdnf5::solv::CompsPool & pool = get_comps_pool(p_impl->base);

    for (auto environment_id : p_impl->environment_ids) {
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
                // If it's successful (wasn't already present), create an XML node for this translation
                if (name_langs.insert(lang).second) {
                    node = utils::xml::add_subnode_with_text(node_environment, "name", std::string(di.kv.str));
                    xmlNewProp(node, BAD_CAST "xml:lang", BAD_CAST lang.c_str());
                }
            }
            // If keyname starts with "solvable:description:", it's a description translation
            else if (keyname.rfind(description_prefix, 0) == 0) {
                lang = keyname.substr(description_prefix.length());
                // Add the lang into the set
                // If it's successful (wasn't already present), create an XML node for this translation
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

void Environment::add_environment_id(const EnvironmentId & environment_id) {
    p_impl->environment_ids.push_back(environment_id);
}

}  // namespace libdnf5::comps
