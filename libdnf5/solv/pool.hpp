// Copyright Contributors to the DNF5 project.
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

#ifndef LIBDNF5_SOLV_POOL_HPP
#define LIBDNF5_SOLV_POOL_HPP

#include "base/base_impl.hpp"
#include "id_queue.hpp"

#include "libdnf5/repo/repo.hpp"

#include <climits>
#include <memory>

extern "C" {
#include <solv/dataiterator.h>
#include <solv/evr.h>
#include <solv/pool.h>
#include <solv/queue.h>
#include <solv/repo.h>
#include <solv/repodata.h>
#include <solv/util.h>
}


namespace libdnf5::solv {

constexpr const char * ZERO_EPOCH = "0";

constexpr const char * SOLVABLE_NAME_ADVISORY_PREFIX = "patch:";
constexpr size_t SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH = std::char_traits<char>::length(SOLVABLE_NAME_ADVISORY_PREFIX);


inline repo::Repo & get_repo(Solvable * solvable) {
    libdnf_assert(solvable->repo->appdata != nullptr, "libsolv repo without libdnf repo set in appdata");
    return *static_cast<repo::Repo *>(solvable->repo->appdata);
}


class Pool;

class TempEvr {
public:
    char * e = nullptr;
    char * v = nullptr;
    char * r = nullptr;

    TempEvr(const Pool & pool, const char * evr);
    ~TempEvr();

    TempEvr(const TempEvr & evr) = delete;
    TempEvr & operator=(const TempEvr & evr) = delete;

    const char * e_def() { return e == nullptr ? ZERO_EPOCH : e; }

private:
    //const Pool & pool;
    char * split_evr = nullptr;
};


class Pool {
public:
    /// Create libsolv pool and set the appropriate pool flags
    Pool() : considered(0) {
        pool = pool_create();
        pool->appdata = this;
        // Ensure excluded packages are not taken as candidates for solver
        pool_set_flag(pool, POOL_FLAG_WHATPROVIDESWITHDISABLED, 1);
        // Allow packages of the same name with different architectures to be installed in parallel
        pool_set_flag(pool, POOL_FLAG_IMPLICITOBSOLETEUSESCOLORS, 1);
    }

    Pool(const Pool & pool) = delete;
    Pool & operator=(const Pool & pool) = delete;

    ~Pool();

    int get_nsolvables() const { return pool->nsolvables; }

    Solvable * id2solvable(Id id) const { return pool_id2solvable(pool, id); }

    const char * id2str(Id id) const { return pool_id2str(pool, id); }

    const char * id2rel(Id id) const { return pool_id2rel(pool, id); }

    const char * id2evr(Id id) const { return pool_id2evr(pool, id); }

    const char * dep2str(Id id) const { return pool_dep2str(pool, id); }

    const char * solvid2str(Id id) const { return pool_solvid2str(pool, id); }

    const char * solvable2str(Solvable * solvable) const { return pool_solvable2str(pool, solvable); }

    Id solvable2id(Solvable * solvable) const { return pool_solvable2id(pool, solvable); }

    Id str2id(const char * str, bool create) const { return pool_str2id(pool, str, create); }

    Id strn2id(const char * str, unsigned int len, bool create) const { return pool_strn2id(pool, str, len, create); }

    Id rel2id(Id name, Id evr, int flags, bool create) const { return pool_rel2id(pool, name, evr, flags, create); }

    Id lookup_id(Id id, Id keyname) const {
        if (id > 0) {
            libdnf5::solv::get_repo(id2solvable(id)).internalize();
        }
        return pool_lookup_id(pool, id, keyname);
    }

    const char * lookup_str(Id id, Id keyname) const {
        if (id > 0) {
            libdnf5::solv::get_repo(id2solvable(id)).internalize();
        }
        return pool_lookup_str(pool, id, keyname);
    }

    unsigned long long lookup_num(Id id, Id keyname) const {
        if (id > 0) {
            libdnf5::solv::get_repo(id2solvable(id)).internalize();
        }
        return pool_lookup_num(pool, id, keyname, 0);
    }

    bool lookup_void(Id id, Id keyname) const {
        if (id > 0) {
            libdnf5::solv::get_repo(id2solvable(id)).internalize();
        }
        return pool_lookup_void(pool, id, keyname);
    }

    const char * get_str_from_pool(Id keyname, Id advisory, int index) const;

    Id queuetowhatprovides(IdQueue & queue) const { return pool_queuetowhatprovides(pool, &queue.get_queue()); }

    int evrcmp(Id evr1, Id evr2, int mode) const { return pool_evrcmp(pool, evr1, evr2, mode); }

    int evrcmp_str(const char * evr1, const char * evr2, int mode) const {
        return pool_evrcmp_str(pool, evr1, evr2, mode);
    }

    /// Split evr into its components.
    ///
    /// Believes blindly in 'evr' being well formed. This could be implemented
    /// without 'pool' of course but either the caller would have to provide buffers
    /// to store the split pieces, or this would call strdup (which is more expensive
    /// than the pool temp space).
    TempEvr split_evr(const char * evr) const { return TempEvr(*this, evr); }


    const char * get_name(Id id) const noexcept { return id2str(id2solvable(id)->name); }

    const char * get_evr(Id id) const noexcept { return id2str(id2solvable(id)->evr); }

    const char * get_epoch(Id id) const noexcept { return split_evr(get_evr(id)).e_def(); }

    unsigned long get_epoch_num(Id id) const;

    const char * get_version(Id id) const noexcept { return split_evr(get_evr(id)).v; }

    const char * get_release(Id id) const noexcept { return split_evr(get_evr(id)).r; }

    const char * get_arch(Id id) const noexcept { return id2str(id2solvable(id)->arch); }

    /// Construct package string ID without epoch when epoch is 0
    /// Returns a temporary object allocated by pool_alloctmpspace
    const char * get_nevra(Id id) const noexcept { return solvable2str(id2solvable(id)); }

    /// Construct package string ID containing always epoch
    std::string get_full_nevra(Id id) const;

    /// Construct package string ID without epoch
    /// Returns a temporary object allocated by pool_alloctmpspace
    const char * get_nevra_without_epoch(Id id) const noexcept;

    /// Construct package string ID containing always epoch
    /// Returns a temporary object allocated by pool_alloctmpspace
    const char * get_nevra_with_epoch(Id id) const noexcept;

    bool is_installed(Solvable * solvable) const { return solvable->repo == pool->installed; }

    bool is_installed(Id id) const { return is_installed(id2solvable(id)); }

    /// Returns `true` if solvable with `id` is excluded.
    /// The return value is undefined for invalid `id`. Valid range for `id` in [1, nsolvables).
    bool is_solvable_excluded(Id id) const { return is_considered_map_active() && !considered.contains(id); }

    repo::Repo & get_repo(Id id) const { return solv::get_repo(id2solvable(id)); }

    const char * get_sourcerpm(Id id) const;


    Id id_to_lowercase_id(const char * name_cstring, bool create) const {
        int name_length = static_cast<int>(strlen(name_cstring));
        auto tmp_name_cstring = pool_alloctmpspace(pool, name_length);
        for (int index = 0; index < name_length; ++index) {
            tmp_name_cstring[index] = static_cast<char>(tolower(name_cstring[index]));
        }
        return pool_strn2id(pool, tmp_name_cstring, static_cast<unsigned int>(name_length), create);
    }

    Id id_to_lowercase_id(Id id_input, bool create) const {
        auto name_cstring = id2str(id_input);
        return id_to_lowercase_id(name_cstring, create);
    }


    bool is_package(Id solvable_id) const {
        Solvable * solvable = id2solvable(solvable_id);
        const char * solvable_name = id2str(solvable->name);
        if (!solvable_name) {
            return true;
        }
        return strncmp(solvable_name, SOLVABLE_NAME_ADVISORY_PREFIX, SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH) != 0;
    }


    ::Pool * operator*() { return &*pool; }
    ::Pool * operator*() const { return &*pool; }

    ::Pool * operator->() { return &*pool; }
    ::Pool * operator->() const { return &*pool; }

    bool is_considered_map_active() const noexcept { return pool->considered; }

    const SolvMap & get_considered_map() const noexcept { return considered; }

    /// Exchanges the internal map `considered` with `other_considered_map`.
    void swap_considered_map(SolvMap & other_considered_map) {
        considered.swap(other_considered_map);
        if (considered.allocated_size() == 0) {
            pool->considered = nullptr;
        } else {
            pool->considered = const_cast<::Map *>(&considered.get_map());
        }
    }

    int set_flag(int flag, int value) { return pool_set_flag(pool, flag, value); }

protected:
    SolvMap considered;  // owner of the considered map, `pool->considered` is only a raw pointer
    ::Pool * pool;
};


class RpmPool : public Pool {
    // TODO(mblaha): Move rpm specific methods from parent Pool class here
};


class CompsPool : public Pool {
public:
    // Search solvables that correspond to the environment_ids for given key
    // Return first non-empty string
    template <typename T>
    std::string lookup_first_id_str(const std::vector<T> & ids, Id key) {
        for (T id : ids) {
            auto value = lookup_str(id.id, key);
            if (value) {
                return value;
            }
        }
        return "";
    }


    template <typename T>
    std::string get_translated_str(const std::vector<T> & ids, Id key, const char * lang = nullptr) {
        // Go through all environment solvables and return first translation found.
        for (T id : ids) {
            Solvable * solvable = id2solvable(id.id);
            const char * translation = nullptr;
            if (lang) {
                translation = solvable_lookup_str_lang(solvable, key, lang, 1);
            } else {
                translation = solvable_lookup_str_poollang(solvable, key);
            }
            if (translation) {
                // Return translation only if it's different from the untranslated string
                // (solvable_lookup_str_lang returns the untranslated string if there is no translation).
                const char * untranslated = solvable_lookup_str(solvable, key);
                if (translation != untranslated && strcmp(translation, untranslated) != 0) {
                    return std::string(translation);
                }
            }
        }
        // If no translation was found, return the untranslated string.
        return lookup_first_id_str(ids, key);
    }

    static std::pair<std::string, std::string> split_solvable_name(std::string_view solvable_name);
};

}  // namespace libdnf5::solv

namespace libdnf5 {

static inline solv::RpmPool & get_rpm_pool(const libdnf5::BaseWeakPtr & base) {
    return InternalBaseUser::get_rpm_pool(base);
}

static inline solv::CompsPool & get_comps_pool(const libdnf5::BaseWeakPtr & base) {
    return InternalBaseUser::get_comps_pool(base);
}

static inline int set_rpm_pool_flag(const libdnf5::BaseWeakPtr & base, int flag, int value) {
    return InternalBaseUser::get_rpm_pool(base).set_flag(flag, value);
}

}  // namespace libdnf5

#endif  // LIBDNF5_SOLV_POOL_HPP
