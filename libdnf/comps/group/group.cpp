#include <libdnf/comps/group/package.hpp>
#include <libdnf/comps/group/group.hpp>
#include <libdnf/comps/group/query.hpp>
#include <libdnf/comps/group/sack.hpp>
#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/comps_impl.hpp>

extern "C" {
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solvable.h>
}

#include <string>
#include <iostream>


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


}  // namespace libdnf::comps
