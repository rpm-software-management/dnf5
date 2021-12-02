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

#include "repo_gpgme.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/logger/logger.hpp"
#include "libdnf/repo/repo_errors.hpp"

#include "utils/bgettext/bgettext-lib.h"
#include "utils/temp.hpp"

#include <fmt/format.h>
#include <gpgme.h>
#include <sys/stat.h>

#include <memory>


// TODO(jrohel) replace gpgme with something else

namespace std {

template <>
struct default_delete<std::remove_pointer<gpgme_ctx_t>::type> {
    void operator()(gpgme_ctx_t ptr) noexcept { gpgme_release(ptr); }
};

}  // namespace std


namespace libdnf::repo {

class Key {
public:
    Key(gpgme_key_t key, gpgme_subkey_t subkey) {
        id = subkey->keyid;
        fingerprint = subkey->fpr;
        timestamp = subkey->timestamp;
        userid = key->uids->uid;
    }

    std::string get_id() const { return id; }
    std::string get_user_id() const { return userid; }
    std::string get_fingerprint() const { return fingerprint; }
    long int get_timestamp() const { return timestamp; }

    std::vector<char> raw_key;

private:
    std::string id;
    std::string fingerprint;
    std::string userid;
    long int timestamp;
};

/// Creates the '/run/user/$UID' directory if it doesn't exist. If this
/// directory exists, gpgagent will create its sockets under
/// '/run/user/$UID/gnupg'.
///
/// If this directory doesn't exist, gpgagent will create its sockets in gpg
/// home directory, which is under '/var/cache/yum/metadata/' and this was
/// causing trouble with container images, see [1].
///
/// Previous solution was to send the agent a "KILLAGENT" message, but that
/// would cause a race condition with calling gpgme_release(), see [2], [3],
/// [4].
///
/// Since the agent doesn't clean up its sockets properly, by creating this
/// directory we make sure they are in a place that is not causing trouble with
/// container images.
///
/// [1] https://bugzilla.redhat.com/show_bug.cgi?id=1650266
/// [2] https://bugzilla.redhat.com/show_bug.cgi?id=1769831
/// [3] https://github.com/rpm-software-management/microdnf/issues/50
/// [4] https://bugzilla.redhat.com/show_bug.cgi?id=1781601
static void ensure_socket_dir_exists(Logger & logger) {
    std::string dirname = "/run/user/" + std::to_string(getuid());
    int res = mkdir(dirname.c_str(), 0700);
    if (res != 0 && errno != EEXIST) {
        logger.debug(fmt::format("Failed to create directory \"{}\": {} - {}", dirname, errno, strerror(errno)));
    }
}


static std::unique_ptr<std::remove_pointer<gpgme_ctx_t>::type> create_context(const std::string & homedir) {
    gpg_error_t gpg_err;
    gpgme_ctx_t ctx;
    gpg_err = gpgme_new(&ctx);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        throw RepoGpgError(M_("Error creating gpgme context: {}"), gpgme_strerror(gpg_err));
    }

    std::unique_ptr<std::remove_pointer<gpgme_ctx_t>::type> context(ctx);

    // set GPG home dir
    gpg_err = gpgme_ctx_set_engine_info(ctx, GPGME_PROTOCOL_OpenPGP, nullptr, homedir.c_str());
    if (gpg_err != GPG_ERR_NO_ERROR) {
        throw RepoGpgError(M_("Failed to set gpgme home directory to \"{}\": {}"), homedir, gpgme_strerror(gpg_err));
    }

    return context;
}


static void gpg_import_key(gpgme_ctx_t context, int key_fd) {
    gpg_error_t gpg_err;
    gpgme_data_t key_data;

    gpg_err = gpgme_data_new_from_fd(&key_data, key_fd);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        throw RepoGpgError(M_("Failed to create gpgme data from file descriptor: {}"), gpgme_strerror(gpg_err));
    }

    gpg_err = gpgme_op_import(context, key_data);
    gpgme_data_release(key_data);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        throw RepoGpgError(M_("Failed to import gpgme keys: {}"), gpgme_strerror(gpg_err));
    }
}


static void gpg_import_key(gpgme_ctx_t context, std::vector<char> key) {
    gpg_error_t gpg_err;
    gpgme_data_t key_data;

    gpg_err = gpgme_data_new_from_mem(&key_data, key.data(), key.size(), 0);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        throw RepoGpgError(M_("Failed to create gpgme data from a buffer: {}"), gpgme_strerror(gpg_err));
    }

    gpg_err = gpgme_op_import(context, key_data);
    gpgme_data_release(key_data);
    if (gpg_err != GPG_ERR_NO_ERROR) {
        throw RepoGpgError(M_("Failed to import gpgme keys: {}"), gpgme_strerror(gpg_err));
    }
}


static std::vector<Key> rawkey2infos(int fd) {
    gpg_error_t gpg_err;

    libdnf::utils::TempDir tmpdir("tmpdir");

    auto context = create_context(tmpdir.get_path());

    gpg_import_key(context.get(), fd);

    std::vector<Key> key_infos;
    gpgme_key_t key;
    gpg_err = gpgme_op_keylist_start(context.get(), nullptr, 0);
    while (gpg_err == GPG_ERR_NO_ERROR) {
        gpg_err = gpgme_op_keylist_next(context.get(), &key);
        if (gpg_err) {
            break;
        }

        // _extract_signing_subkey
        auto subkey = key->subkeys;
        while (subkey && !subkey->can_sign) {
            subkey = subkey->next;
        }
        if (subkey)
            key_infos.emplace_back(key, subkey);
        gpgme_key_release(key);
    }
    if (gpg_err_code(gpg_err) != GPG_ERR_EOF) {
        throw RepoGpgError(M_("Failed to list gpg keys: {}"), gpgme_strerror(gpg_err));
    }
    gpgme_set_armor(context.get(), 1);
    for (auto & key_info : key_infos) {
        gpgme_data_t sink;
        gpgme_data_new(&sink);
        gpgme_op_export(context.get(), key_info.get_id().c_str(), 0, sink);
        gpgme_data_rewind(sink);

        char buf[4096];
        ssize_t readed;
        do {
            readed = gpgme_data_read(sink, buf, sizeof(buf));
            if (readed > 0)
                key_info.raw_key.insert(key_info.raw_key.end(), buf, buf + sizeof(buf));
        } while (readed == sizeof(buf));
    }
    return key_infos;
}


RepoGpgme::RepoGpgme(const BaseWeakPtr & base, const ConfigRepo & config) : base(base), config(config) {
    auto & logger = *base->get_logger();

    ensure_socket_dir_exists(logger);

    gpg_error_t gpg_err;

    gpgme_check_version(nullptr);

    struct stat sb;
    if (stat(get_keyring_dir().c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        auto context = create_context(get_keyring_dir());

        gpgme_key_t key;
        gpg_err = gpgme_op_keylist_start(context.get(), nullptr, 0);
        while (gpg_err == GPG_ERR_NO_ERROR) {
            gpg_err = gpgme_op_keylist_next(context.get(), &key);
            if (gpg_err) {
                break;
            }

            // _extract_signing_subkey
            auto subkey = key->subkeys;
            while (subkey && !key->subkeys->can_sign) {
                subkey = subkey->next;
            }
            if (subkey) {
                known_keys.push_back(subkey->keyid);
            }
            gpgme_key_release(key);
        }

        if (gpg_err_code(gpg_err) != GPG_ERR_EOF) {
            throw RepoGpgError(M_("Failed to list gpg keys: {}"), gpgme_strerror(gpg_err));
        }
    }
}


void RepoGpgme::import_key(int fd, const std::string & url) {
    auto & logger = *base->get_logger();

    auto key_infos = rawkey2infos(fd);

    for (auto & key_info : key_infos) {
        if (std::find(known_keys.begin(), known_keys.end(), key_info.get_id()) != known_keys.end()) {
            logger.debug(fmt::format("Gpg key 0x{} for repository {} already imported.", key_info.get_id(), config.get_id()));
            continue;
        }

        if (callbacks) {
            if (!callbacks->repokey_import(
                    key_info.get_id(),
                    key_info.get_user_id(),
                    key_info.get_fingerprint(),
                    url,
                    key_info.get_timestamp()))
                continue;
        }

        struct stat sb;
        if (stat(get_keyring_dir().c_str(), &sb) != 0 || !S_ISDIR(sb.st_mode))
            mkdir(get_keyring_dir().c_str(), 0777);

        auto context = create_context(get_keyring_dir());

        gpg_import_key(context.get(), key_info.raw_key);

        logger.debug(fmt::format("Imported gpg key 0x{} for repository {}.", key_info.get_id(), config.get_id()));
    }
}

}  // namespace libdnf::repo
