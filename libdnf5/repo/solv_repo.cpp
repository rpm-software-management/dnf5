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

#include "solv_repo.hpp"

#include "base/base_impl.hpp"
#include "repo_cache_private.hpp"
#include "solv/pool.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/temp.hpp"
#include "libdnf5/utils/to_underlying.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/repo_comps.h>
#include <solv/repo_deltainfoxml.h>
#include <solv/repo_repomdxml.h>
#include <solv/repo_rpmdb.h>
#include <solv/repo_rpmmd.h>
#include <solv/repo_solv.h>
#include <solv/repo_updateinfoxml.h>
#include <solv/repo_write.h>
}


namespace libdnf5::repo {

namespace fs = libdnf5::utils::fs;


constexpr auto CHKSUM_TYPE = REPOKEY_TYPE_SHA256;
constexpr const char * CHKSUM_IDENT = "H000";


static std::array<char, SOLV_USERDATA_SOLV_TOOLVERSION_SIZE> get_padded_solv_toolversion() {
    std::array<char, SOLV_USERDATA_SOLV_TOOLVERSION_SIZE> padded_solv_toolversion{};
    std::string solv_ver_str{solv_toolversion};
    std::copy(solv_ver_str.rbegin(), solv_ver_str.rend(), padded_solv_toolversion.rbegin());

    return padded_solv_toolversion;
}

void SolvRepo::userdata_fill(SolvUserdata * userdata) {
    if (strlen(solv_toolversion) > SOLV_USERDATA_SOLV_TOOLVERSION_SIZE) {
        libdnf_throw_assertion(
            "Libsolv's solv_toolvesion is: {} long but we expect max of: {}",
            strlen(solv_toolversion),
            SOLV_USERDATA_SOLV_TOOLVERSION_SIZE);
    }

    memcpy(userdata->dnf_magic, SOLV_USERDATA_MAGIC.data(), SOLV_USERDATA_MAGIC.size());
    memcpy(userdata->dnf_version, SOLV_USERDATA_DNF_VERSION.data(), SOLV_USERDATA_DNF_VERSION.size());
    memcpy(userdata->libsolv_version, get_padded_solv_toolversion().data(), SOLV_USERDATA_SOLV_TOOLVERSION_SIZE);
    memcpy(userdata->checksum, checksum, CHKSUM_BYTES);
}

bool SolvRepo::can_use_solvfile_cache(solv::Pool & pool, fs::File & solvfile_cache) {
    auto & logger = *base->get_logger();

    if (!solvfile_cache) {
        logger.debug(("Missing solvfile cache: \"{}\""), solvfile_cache.get_path().native());
        return false;
    }

    unsigned char * dnf_solv_userdata_read;
    int dnf_solv_userdata_len_read;

    int ret_code = solv_read_userdata(solvfile_cache.get(), &dnf_solv_userdata_read, &dnf_solv_userdata_len_read);
    std::unique_ptr<SolvUserdata, decltype(&solv_free)> solv_userdata(
        reinterpret_cast<SolvUserdata *>(dnf_solv_userdata_read), &solv_free);
    if (ret_code != 0) {
        logger.warning(
            ("Failed to read solv userdata: \"{}\": for: {}"), pool_errstr(*pool), solvfile_cache.get_path().native());
        return false;
    }

    if (dnf_solv_userdata_len_read != SOLV_USERDATA_SIZE) {
        logger.warning(
            ("Solv userdata length mismatch read: \"{}\" vs expected \"{}\" for: {}"),
            dnf_solv_userdata_len_read,
            SOLV_USERDATA_SIZE,
            solvfile_cache.get_path().native());
        return false;
    }

    // check dnf solvfile magic bytes
    if (memcmp(solv_userdata->dnf_magic, SOLV_USERDATA_MAGIC.data(), SOLV_USERDATA_MAGIC.size()) != 0) {
        logger.warning(
            "Magic bytes don't match, read: \"{}\" vs. dnf solvfile magic: \"{}\" for: {}",
            solv_userdata->dnf_magic,
            SOLV_USERDATA_MAGIC.data(),
            solvfile_cache.get_path().native());
        return false;
    }

    // check dnf solvfile version
    if (memcmp(solv_userdata->dnf_version, SOLV_USERDATA_DNF_VERSION.data(), SOLV_USERDATA_DNF_VERSION.size()) != 0) {
        logger.warning(
            "Dnf solvfile version doesn't match, read: \"{}\" vs. expected dnf solvfile version: \"{}\" for: {}",
            solv_userdata->dnf_version,
            SOLV_USERDATA_DNF_VERSION.data(),
            solvfile_cache.get_path().native());
        return false;
    }

    // check libsolv solvfile version
    if (memcmp(
            solv_userdata->libsolv_version,
            get_padded_solv_toolversion().data(),
            SOLV_USERDATA_SOLV_TOOLVERSION_SIZE) != 0) {
        logger.warning(
            "Libsolv solvfile version doesn't match, read: \"{}\" vs. expected libsolv version: \"{}\" for: {}",
            solv_userdata->libsolv_version,
            solv_toolversion,
            solvfile_cache.get_path().native());
        return false;
    }

    // check solvfile checksum
    if (memcmp(solv_userdata->checksum, checksum, CHKSUM_BYTES) != 0) {
        logger.debug(
            "Solvfile's repomd checksum doesn't match, read: \"{}\" vs. expected repomd checksum: \"{}\" for: {}",
            pool_bin2hex(*pool, solv_userdata->checksum, sizeof solv_userdata->checksum),
            pool_bin2hex(*pool, checksum, sizeof checksum),
            solvfile_cache.get_path().native());
        return false;
    }

    solvfile_cache.rewind();
    return true;
}


// Computes checksum of data in opened file.
// Calls rewind(fp) before returning.
void checksum_calc(unsigned char * out, fs::File & file) {
    // based on calc_checksum_fp in libsolv's solv.c
    char buf[4096];
    auto h = solv_chksum_create(CHKSUM_TYPE);
    int l;

    file.rewind();
    solv_chksum_add(h, CHKSUM_IDENT, strlen(CHKSUM_IDENT));
    while ((l = static_cast<int>(file.read(buf, sizeof(buf)))) > 0) {
        solv_chksum_add(h, buf, l);
    }
    file.rewind();
    solv_chksum_free(h, out);
}


static const char * repodata_type_to_name(RepodataType type) {
    switch (type) {
        case RepodataType::FILELISTS:
            return RepoDownloader::MD_FILENAME_FILELISTS;
        case RepodataType::PRESTO:
            return RepoDownloader::MD_FILENAME_PRESTODELTA;
        case RepodataType::UPDATEINFO:
            return RepoDownloader::MD_FILENAME_UPDATEINFO;
        case RepodataType::COMPS:
            return RepoDownloader::MD_FILENAME_GROUP;
        case RepodataType::OTHER:
            return RepoDownloader::MD_FILENAME_OTHER;
    }

    libdnf_throw_assertion("Unknown RepodataType: {}", utils::to_underlying(type));
}


static int repodata_type_to_flags(RepodataType type) {
    switch (type) {
        case RepodataType::FILELISTS:
            return REPO_EXTEND_SOLVABLES | REPO_LOCALPOOL;
        case RepodataType::PRESTO:
            return REPO_EXTEND_SOLVABLES;
        case RepodataType::UPDATEINFO:
            return 0;
        case RepodataType::COMPS:
            return 0;
        case RepodataType::OTHER:
            return REPO_EXTEND_SOLVABLES | REPO_LOCALPOOL;
    }

    libdnf_throw_assertion("Unknown RepodataType: {}", utils::to_underlying(type));
}


// Returns `true` when all solvables in the repository are stored continuously,
// without interleaving with solvables from other repositories.
// Complexity: Linear to the current number of solvables in the repository
bool is_one_piece(::Repo * repo) {
    for (auto i = repo->start; i < repo->end; ++i) {
        if (repo->pool->solvables[i].repo != repo) {
            return false;
        }
    }

    return true;
}


SolvRepo::SolvRepo(const libdnf5::BaseWeakPtr & base, const ConfigRepo & config, void * appdata)
    : base(base),
      config(config),
      repo(repo_create(*get_rpm_pool(base), config.get_id().c_str())),
      comps_repo(repo_create(*get_comps_pool(base), config.get_id().c_str())) {
    repo->appdata = appdata;
    comps_repo->appdata = appdata;
}


SolvRepo::~SolvRepo() {
    repo->appdata = nullptr;
    comps_repo->appdata = nullptr;
}


void SolvRepo::load_repo_main(const std::string & repomd_fn, const std::string & primary_fn) {
    auto & logger = *base->get_logger();
    auto & pool = get_rpm_pool(base);

    fs::File repomd_file(repomd_fn, "r");

    checksum_calc(checksum, repomd_file);

    int solvables_start = pool->nsolvables;

    if (load_solv_cache(pool, nullptr, 0)) {
        main_solvables_start = solvables_start;
        main_solvables_end = pool->nsolvables;

        return;
    }

    fs::File primary_file(primary_fn, "r", true);

    logger.debug("Loading repomd and primary for repo \"{}\"", config.get_id());
    if (repo_add_repomdxml(repo, repomd_file.get(), 0) != 0) {
        throw SolvError(
            M_("Failed to load repomd for repo \"{}\" from \"{}\": {}."),
            config.get_id(),
            repomd_fn,
            std::string(pool_errstr(*pool)));
    }

    if (repo_add_rpmmd(repo, primary_file.get(), 0, 0) != 0) {
        throw SolvError(
            M_("Failed to load primary for repo \"{}\" from \"{}\": {}."),
            config.get_id(),
            primary_fn,
            std::string(pool_errstr(*pool)));
    }

    main_solvables_start = solvables_start;
    main_solvables_end = pool->nsolvables;

    if (config.get_build_cache_option().get_value()) {
        write_main(true);
    }
}


void SolvRepo::load_system_repo_ext(RepodataType type) {
    std::string type_name = repodata_type_to_name(type);
    switch (type) {
        case RepodataType::COMPS: {
            // get installed groups from system state and load respective xml files
            // to the libsolv pool
            auto & system_state = base->p_impl->get_system_state();
            auto comps_dir = system_state.get_group_xml_dir();
            for (auto & group_id : system_state.get_installed_groups()) {
                auto ext_fn = comps_dir / (group_id + ".xml");
                if (!read_group_solvable_from_xml(ext_fn)) {
                    // The group xml file either not exists or is not parseable by
                    // libsolv.
                    groups_missing_xml.push_back(std::move(group_id));
                }
            }
            for (auto & environment_id : system_state.get_installed_environments()) {
                auto ext_fn = comps_dir / (environment_id + ".xml");
                if (!read_group_solvable_from_xml(ext_fn)) {
                    // The environment xml file either not exists or is not parseable by
                    // libsolv.
                    environments_missing_xml.push_back(std::move(environment_id));
                }
            }
            break;
        }
        case RepodataType::FILELISTS:
        case RepodataType::OTHER:
        case RepodataType::PRESTO:
        case RepodataType::UPDATEINFO:
            throw SolvError(M_("Unsupported extended repodata type for the system repo: \"{}\"."), type_name);
    }
}


void SolvRepo::load_repo_ext(RepodataType type, const RepoDownloader & downloader) {
    auto & logger = *base->get_logger();
    solv::Pool & pool = type == RepodataType::COMPS ? static_cast<solv::Pool &>(get_comps_pool(base))
                                                    : static_cast<solv::Pool &>(get_rpm_pool(base));

    std::string type_name = repodata_type_to_name(type);

    std::string ext_fn;

    if (type == RepodataType::COMPS) {
        ext_fn = downloader.get_metadata_path(RepoDownloader::MD_FILENAME_GROUP_GZ);
        if (ext_fn.empty()) {
            ext_fn = downloader.get_metadata_path(type_name);
        }
    } else {
        ext_fn = downloader.get_metadata_path(type_name);
    }

    if (ext_fn.empty()) {
        logger.debug("No {} metadata available for repo \"{}\"", type_name, config.get_id());
        return;
    }

    int solvables_start = pool->nsolvables;

    if (load_solv_cache(pool, type_name.c_str(), repodata_type_to_flags(type))) {
        if (type == RepodataType::UPDATEINFO) {
            updateinfo_solvables_start = solvables_start;
            updateinfo_solvables_end = pool->nsolvables;
        }

        return;
    }

    fs::File ext_file(ext_fn, "r", true);
    logger.debug("Loading {} extension for repo \"{}\" from \"{}\"", type_name, config.get_id(), ext_fn);

    int res = 0;
    switch (type) {
        case RepodataType::FILELISTS:
            res = repo_add_rpmmd(repo, ext_file.get(), "FL", REPO_EXTEND_SOLVABLES);
            break;
        case RepodataType::PRESTO:
            res = repo_add_deltainfoxml(repo, ext_file.get(), 0);
            break;
        case RepodataType::UPDATEINFO:
            if ((res = repo_add_updateinfoxml(repo, ext_file.get(), 0)) == 0) {
                updateinfo_solvables_start = solvables_start;
                updateinfo_solvables_end = pool->nsolvables;
            }
            break;
        case RepodataType::COMPS:
            res = repo_add_comps(comps_repo, ext_file.get(), 0);
            break;
        case RepodataType::OTHER:
            res = repo_add_rpmmd(repo, ext_file.get(), 0, REPO_EXTEND_SOLVABLES);
            break;
    }

    if (res != 0) {
        throw SolvError(
            M_("Failed to load {} extension for repo \"{}\" from \"{}\": {}"),
            type_name,
            config.get_id(),
            ext_fn,
            std::string(pool_errstr(*get_rpm_pool(base))));
    }

    if (config.get_build_cache_option().get_value()) {
        if (type == RepodataType::COMPS) {
            write_ext(comps_repo->nrepodata - 1, type);
        } else {
            write_ext(repo->nrepodata - 1, type);
        }
    }
}


void SolvRepo::load_system_repo(const std::string & rootdir) {
    auto & logger = *base->get_logger();
    auto & pool = get_rpm_pool(base);

    logger.debug("Loading system repo rpmdb from root \"{}\"", rootdir.empty() ? "/" : rootdir);
    if (rootdir.empty()) {
        base->get_config().get_installroot_option().lock("installroot locked by loading system repo");
        pool_set_rootdir(*pool, base->get_config().get_installroot_option().get_value().c_str());
    } else {
        pool_set_rootdir(*pool, rootdir.c_str());
    }

    int solvables_start = pool->nsolvables;

    // TODO(egoode) investigate performance hit of RPM_ADD_WITH_CHANGELOG, possibly make this configurable
    int flagsrpm = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR | RPM_ADD_WITH_CHANGELOG;
    if (repo_add_rpmdb(repo, nullptr, flagsrpm) != 0) {
        throw SolvError(
            M_("Failed to load system repo from root \"{}\": {}"),
            rootdir.empty() ? "/" : rootdir,
            std::string(pool_errstr(*get_rpm_pool(base))));
    }

    if (!rootdir.empty()) {
        // if loading an extra repo, reset rootdir back to installroot
        pool_set_rootdir(*pool, base->get_config().get_installroot_option().get_value().c_str());
    }

    pool_set_installed(*pool, repo);

    main_solvables_start = solvables_start;
    main_solvables_end = pool->nsolvables;
}


// return true if q1 is a superset of q2
// only works if there are no duplicates both in q1 and q2
// the map parameter must point to an empty map that can hold all ids
// (it is also returned empty)
static bool is_superset(
    const libdnf5::solv::IdQueue & q1, const libdnf5::solv::IdQueue & q2, libdnf5::solv::SolvMap & map) {
    int cnt = 0;
    for (int i = 0; i < q2.size(); i++) {
        map.add_unsafe(q2[i]);
    }
    for (int i = 0; i < q1.size(); i++) {
        if (map.contains_unsafe(q1[i])) {
            cnt++;
        }
    }
    for (int i = 0; i < q2.size(); i++) {
        map.remove_unsafe(q2[i]);
    }
    return cnt == q2.size();
}


void SolvRepo::rewrite_repo(libdnf5::solv::IdQueue & fileprovides) {
    auto & logger = *base->get_logger();
    auto & pool = get_rpm_pool(base);

    logger.debug("Rewriting repo \"{}\" with added file provides", config.get_id());

    if (!config.get_build_cache_option().get_value() || main_solvables_start == 0 || fileprovides.size() == 0) {
        return;
    }

    libdnf5::solv::IdQueue fileprovidesq;
    libdnf5::solv::SolvMap providedids(pool->ss.nstrings);
    Repodata * data = repo_id2repodata(repo, 1);
    if (repodata_lookup_idarray(data, SOLVID_META, REPOSITORY_ADDEDFILEPROVIDES, &fileprovidesq.get_queue())) {
        if (is_superset(fileprovidesq, fileprovides, providedids)) {
            return;
        }
    }

    repodata_set_idarray(data, SOLVID_META, REPOSITORY_ADDEDFILEPROVIDES, &fileprovides.get_queue());
    repodata_internalize(data);

    write_main(false);
}


void SolvRepo::internalize() {
    if (!needs_internalizing) {
        return;
    }
    repo_internalize(repo);
    needs_internalizing = false;
}


void SolvRepo::set_priority(int priority) {
    repo->priority = priority;
}


void SolvRepo::set_subpriority(int subpriority) {
    repo->subpriority = subpriority;
}


bool SolvRepo::load_solv_cache(solv::Pool & pool, const char * type_name, int flags) {
    auto & logger = *base->get_logger();

    auto path = solv_file_path(type_name);

    try {
        fs::File cache_file(path, "r");

        if (can_use_solvfile_cache(pool, cache_file)) {
            logger.debug("Loading solv cache file: \"{}\"", path.native());
            if (repo_add_solv(
                    type_name && std::string_view(type_name) == RepoDownloader::MD_FILENAME_GROUP ? comps_repo : repo,
                    cache_file.get(),
                    flags) != 0) {
                throw SolvError(
                    M_("Failed to load {} cache for repo \"{}\" from \"{}\": {}"),
                    type_name ? std::string(type_name) : "primary",
                    config.get_id(),
                    path.native(),
                    std::string(pool_errstr(*get_rpm_pool(base))));
            }
            return true;
        }
    } catch (const FileSystemError & e) {
        if (std::error_code(e.get_error_code(), std::system_category()).default_error_condition() ==
            std::errc::no_such_file_or_directory) {
            logger.trace("Cache file \"{}\" not found", path.native());
        } else {
            logger.warning("Error opening cache file, ignoring: {}", e.what());
        }
    }

    return false;
}


void SolvRepo::write_main(bool load_after_write) {
    auto & logger = *base->get_logger();
    auto & pool = get_rpm_pool(base);

    const char * chksum = pool_bin2hex(*pool, checksum, solv_chksum_len(CHKSUM_TYPE));

    const auto solvfile_path = solv_file_path();
    const auto solvfile_parent_dir = solvfile_path.parent_path();

    std::filesystem::create_directory(solvfile_parent_dir);

    auto cache_tmp_file = fs::TempFile(solvfile_parent_dir, solvfile_path.filename());
    auto & cache_file = cache_tmp_file.open_as_file("w+");

    logger.trace(
        "Writing primary cache for repo \"{}\" to \"{}\" (checksum: 0x{})",
        config.get_id(),
        cache_tmp_file.get_path().native(),
        chksum);

    SolvUserdata solv_userdata{};
    userdata_fill(&solv_userdata);

    Repowriter * writer = repowriter_create(repo);
    repowriter_set_userdata(writer, &solv_userdata, SOLV_USERDATA_SIZE);
    repowriter_set_solvablerange(writer, main_solvables_start, main_solvables_end);
    int res = repowriter_write(writer, cache_file.get());
    repowriter_free(writer);

    if (res != 0) {
        throw SolvError(
            M_("Failed to write primary cache for repo \"{}\" to \"{}\": {}"),
            config.get_id(),
            cache_tmp_file.get_path().native(),
            std::string(pool_errstr(*pool)));
    }

    cache_tmp_file.close();

    if (load_after_write && is_one_piece(repo)) {
        // this saves memory, libsolv doesn't load all the data from a solv file, it dup()s the fd,
        // keeps the file open and lazily loads some data on-demand.
        fs::File file(cache_tmp_file.get_path(), "r");

        repo_empty(repo, 1);
        int ret = repo_add_solv(repo, file.get(), 0);
        if (ret) {
            throw SolvError(
                M_("Failed to re-load primary cache for repo \"{}\" from \"{}\": {}"),
                config.get_id(),
                cache_tmp_file.get_path().native(),
                std::string(pool_errstr(*pool)));
        }
    }

    std::filesystem::permissions(
        cache_tmp_file.get_path(),
        std::filesystem::perms::group_read | std::filesystem::perms::others_read,
        std::filesystem::perm_options::add);
    std::filesystem::rename(cache_tmp_file.get_path(), solvfile_path);
    cache_tmp_file.release();
}


void SolvRepo::write_ext(Id repodata_id, RepodataType type) {
    libdnf_assert(repodata_id != 0, "0 is not a valid repodata id");

    auto & logger = *base->get_logger();
    solv::Pool & pool = type == RepodataType::COMPS ? static_cast<solv::Pool &>(get_comps_pool(base))
                                                    : static_cast<solv::Pool &>(get_rpm_pool(base));

    const std::string type_name = repodata_type_to_name(type);
    const auto solvfile_path = solv_file_path(type_name.c_str());
    const auto solvfile_parent_dir = solvfile_path.parent_path();

    std::filesystem::create_directory(solvfile_parent_dir);

    auto cache_tmp_file = fs::TempFile(solvfile_parent_dir, solvfile_path.filename());
    auto & cache_file = cache_tmp_file.open_as_file("w+");

    logger.trace(
        "Writing {} extension cache for repo \"{}\" to \"{}\"",
        type_name,
        config.get_id(),
        cache_tmp_file.get_path().native());


    SolvUserdata solv_userdata{};
    userdata_fill(&solv_userdata);

    Repowriter * writer;
    if (type == RepodataType::COMPS) {
        writer = repowriter_create(comps_repo);
    } else {
        writer = repowriter_create(repo);
    }
    repowriter_set_userdata(writer, &solv_userdata, SOLV_USERDATA_SIZE);
    repowriter_set_repodatarange(writer, repodata_id, repodata_id + 1);

    if (type == RepodataType::UPDATEINFO) {
        repowriter_set_solvablerange(writer, updateinfo_solvables_start, updateinfo_solvables_end);
    }

    if (type != RepodataType::COMPS && type != RepodataType::UPDATEINFO) {
        repowriter_set_flags(writer, REPOWRITER_NO_STORAGE_SOLVABLE);
    }

    int res = repowriter_write(writer, cache_file.get());
    repowriter_free(writer);

    if (res != 0) {
        throw SolvError(
            M_("Failed to write {} cache for repo \"{}\" to \"{}\": {}"),
            type_name,
            config.get_id(),
            cache_tmp_file.get_path().native(),
            std::string(pool_errstr(*pool)));
    }

    cache_tmp_file.close();

    if (is_one_piece(repo) && type != RepodataType::UPDATEINFO && type != RepodataType::COMPS) {
        // this saves memory, libsolv doesn't load all the data from a solv file, it dup()s the fd,
        // keeps the file open and lazily loads some data on-demand.
        fs::File file(cache_tmp_file.get_path(), "r");

        Repodata * data = repo_id2repodata(repo, repodata_id);

        repodata_extend_block(data, repo->start, repo->end - repo->start);
        data->state = REPODATA_LOADING;
        res = repo_add_solv(repo, file.get(), repodata_type_to_flags(type) | REPO_USE_LOADING);
        if (res) {
            throw SolvError(
                M_("Failed to re-load {} cache for repo \"{}\" from \"{}\": {}"),
                type_name,
                config.get_id(),
                cache_tmp_file.get_path().native(),
                std::string(pool_errstr(*pool)));
        }
        data->state = REPODATA_AVAILABLE;
    }

    std::filesystem::permissions(
        cache_tmp_file.get_path(),
        std::filesystem::perms::group_read | std::filesystem::perms::others_read,
        std::filesystem::perm_options::add);
    std::filesystem::rename(cache_tmp_file.get_path(), solvfile_path);
    cache_tmp_file.release();
}


std::string SolvRepo::solv_file_name(const char * type) {
    if (type != nullptr) {
        return fmt::format("{}-{}.solvx", config.get_id(), type);
    } else {
        return config.get_id() + ".solv";
    }
}


std::filesystem::path SolvRepo::solv_file_path(const char * type) {
    return std::filesystem::path(config.get_cachedir()) / CACHE_SOLV_FILES_DIR / solv_file_name(type);
}

bool SolvRepo::read_group_solvable_from_xml(const std::string & path) {
    auto & logger = *base->get_logger();
    bool read_success = true;

    fs::File ext_file;
    try {
        ext_file = fs::File(path, "r", true);
    } catch (FileSystemError & e) {
        logger.warning("Cannot load group extension for {} repo from \"{}\": {}", config.get_id(), path, e.what());
        read_success = false;
    }

    if (read_success) {
        logger.debug("Loading group extension for {} repo from \"{}\"", config.get_id(), path);
        read_success = repo_add_comps(comps_repo, ext_file.get(), 0) == 0;
        if (!read_success) {
            logger.debug("Loading group extension for {} repo from \"{}\" failed.", config.get_id(), path);
        }
    }

    return read_success;
}

void SolvRepo::create_group_solvable(const std::string & groupid, const libdnf5::system::GroupState & state) {
    solv::Pool & pool = static_cast<solv::Pool &>(get_comps_pool(base));
    libdnf_assert(
        comps_repo == (*pool)->installed, "SolvRepo::create_group_solvable() call enabled only for @System repo.");

    // create a new solvable for the group
    auto group_solvable_id = repo_add_solvable(comps_repo);
    Solvable * group_solvable = pool.id2solvable(group_solvable_id);

    // Information about group contained in the system state is very limited.
    // We have only repoid and list of installed packages.

    // Set id with proper prefix
    group_solvable->name = pool.str2id(("group:" + groupid).c_str(), 1);
    // Set noarch and empty evr
    group_solvable->arch = ARCH_NOARCH;
    group_solvable->evr = ID_EMPTY;
    // Make the new group provide it's own id
    group_solvable->dep_provides = repo_addid_dep(
        comps_repo, group_solvable->dep_provides, pool.rel2id(group_solvable->name, group_solvable->evr, REL_EQ, 1), 0);

    // Add group packages
    for (const auto & pkg_name : state.packages) {
        auto pkg_id = pool.str2id(pkg_name.c_str(), 1);
        Id type = SOLVABLE_RECOMMENDS;
        repo_add_idarray(comps_repo, group_solvable_id, type, pkg_id);
    }

    Repodata * data = repo_last_repodata(comps_repo);

    // Mark the repo as user-visible
    repodata_set_void(data, group_solvable_id, SOLVABLE_ISVISIBLE);

    repodata_internalize(data);
}

void SolvRepo::create_environment_solvable(
    const std::string & environmentid, const libdnf5::system::EnvironmentState & state) {
    solv::Pool & pool = static_cast<solv::Pool &>(get_comps_pool(base));
    libdnf_assert(
        comps_repo == (*pool)->installed,
        "SolvRepo::create_environment_solvable() call enabled only for @System repo.");

    // create a new solvable for the environment
    auto environment_solvable_id = repo_add_solvable(comps_repo);
    Solvable * environment_solvable = pool.id2solvable(environment_solvable_id);

    // Information about environment contained in the system state is very limited.
    // We have only repoid and list of installed groups.

    // Set id with proper prefix
    environment_solvable->name = pool.str2id(("environment:" + environmentid).c_str(), 1);
    // Set noarch and empty evr
    environment_solvable->arch = ARCH_NOARCH;
    environment_solvable->evr = ID_EMPTY;
    // Make the new environment provide it's own id
    environment_solvable->dep_provides = repo_addid_dep(
        comps_repo,
        environment_solvable->dep_provides,
        pool.rel2id(environment_solvable->name, environment_solvable->evr, REL_EQ, 1),
        0);

    // Add groups in the environment
    for (const auto & grp_name : state.groups) {
        auto grp_id = pool.str2id(grp_name.c_str(), 1);
        repo_add_idarray(comps_repo, environment_solvable_id, SOLVABLE_REQUIRES, grp_id);
    }

    Repodata * data = repo_last_repodata(comps_repo);

    // Mark the repo as user-visible
    repodata_set_void(data, environment_solvable_id, SOLVABLE_ISVISIBLE);

    repodata_internalize(data);
}

void SolvRepo::empty() {
    // TODO(mblaha): is resuseid=true safe enough?
    repo_empty(repo, true);
}

}  // namespace libdnf5::repo
