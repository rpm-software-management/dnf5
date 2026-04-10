// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "utils/string.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <fcntl.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/plugin/iplugin.hpp>
#include <libdnf5/repo/repo.hpp>
#include <libdnf5/repo/repo_query.hpp>
#include <sys/wait.h>


constexpr const char * LOCAL_REPO_NAME_GPGCHECK{"_dnf_local"};
constexpr const char * LOCAL_REPO_NAME_NOGPGCHECK{"_dnf_local_nogpgcheck"};
constexpr const char * LOCATION_IN_PERSISTDIR_GPGCHECK{"plugins/local"};
constexpr const char * LOCATION_IN_PERSISTDIR_NOGPGCHECK{"plugins/local-nogpgcheck"};

using namespace libdnf5;

namespace {

constexpr const char * PLUGIN_NAME{"local"};
constexpr libdnf5::plugin::Version PLUGIN_VERSION{.major = 1, .minor = 0, .micro = 0};
constexpr PluginAPIVersion REQUIRED_PLUGIN_API_VERSION{.major = 2, .minor = 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{
    "Aleš Matěj",
    "amatej@redhat.com",
    "Automatically copy all downloaded packages to a repository configured path and generate repo metadata."};


class LocalPluginError : public libdnf5::Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::plugin"; }
    const char * get_name() const noexcept override { return "LocalPluginError"; }
};


class LocalPlugin : public plugin::IPlugin {
public:
    LocalPlugin(libdnf5::plugin::IPluginData & data, libdnf5::ConfigParser & config) : IPlugin(data), config(config) {}
    virtual ~LocalPlugin() = default;

    PluginAPIVersion get_api_version() const noexcept override { return REQUIRED_PLUGIN_API_VERSION; }

    const char * get_name() const noexcept override { return PLUGIN_NAME; }

    plugin::Version get_version() const noexcept override { return PLUGIN_VERSION; }

    const char * const * get_attributes() const noexcept override { return attrs; }

    const char * get_attribute(const char * attribute) const noexcept override {
        for (size_t i = 0; attrs[i]; ++i) {
            if (std::strcmp(attribute, attrs[i]) == 0) {
                return attrs_value[i];
            }
        }
        return nullptr;
    }

    void post_base_setup() override {
        Base & base = get_base();
        auto repo_sack = base.get_repo_sack();
        if (config.has_option("main", "repodir")) {
            repodir = config.get_value("main", "repodir");
        } else {
            // default repo location
            repodir = base.get_config().get_persistdir_option().get_value();
            repodir = repodir / LOCATION_IN_PERSISTDIR_GPGCHECK;
        }
        if (config.has_option("main", "repodir_nogpgcheck")) {
            repodir_nogpgcheck = config.get_value("main", "repodir_nogpgcheck");
        } else {
            repodir_nogpgcheck = base.get_config().get_persistdir_option().get_value();
            repodir_nogpgcheck = repodir_nogpgcheck / LOCATION_IN_PERSISTDIR_NOGPGCHECK;
        }

        // Only create repos if their directories already exist.
        // Directories are created on first use in post_transaction().
        // This avoids warnings about missing repodata (see issue #2581).
        if (std::filesystem::exists(repodir)) {
            setup_local_repo(repo_sack, LOCAL_REPO_NAME_GPGCHECK, "Local libdnf5 plugin repo", repodir);
        }
        if (std::filesystem::exists(repodir_nogpgcheck)) {
            auto nogpgcheck_repo = setup_local_repo(
                repo_sack, LOCAL_REPO_NAME_NOGPGCHECK, "Local libdnf5 plugin repo (nogpgcheck)", repodir_nogpgcheck);
            nogpgcheck_repo->get_config().get_pkg_gpgcheck_option().set(false);
            // Prefer the gpgcheck repo when both contain the same package
            nogpgcheck_repo->get_config().get_cost_option().set(501);
        }
    }

    void post_transaction(const libdnf5::base::Transaction & transaction) override {
        bool need_rebuild_gpgcheck = false;
        bool need_rebuild_nogpgcheck = false;
        for (auto & tspkg : transaction.get_transaction_packages()) {
            if (transaction_item_action_is_inbound(tspkg.get_action())) {
                const auto & pkg = tspkg.get_package();
                if (pkg.get_repo_id() == LOCAL_REPO_NAME_GPGCHECK || pkg.get_repo_id() == LOCAL_REPO_NAME_NOGPGCHECK) {
                    continue;
                }
                if (pkg.is_pkg_gpgcheck_enabled()) {
                    if (!need_rebuild_gpgcheck) {
                        std::filesystem::create_directories(repodir);
                    }
                    std::filesystem::copy(
                        pkg.get_package_path(), repodir, std::filesystem::copy_options::overwrite_existing);
                    need_rebuild_gpgcheck = true;
                } else {
                    if (!need_rebuild_nogpgcheck) {
                        std::filesystem::create_directories(repodir_nogpgcheck);
                    }
                    std::filesystem::copy(
                        pkg.get_package_path(), repodir_nogpgcheck, std::filesystem::copy_options::overwrite_existing);
                    need_rebuild_nogpgcheck = true;
                }
            }
        }

        bool createrepo_enabled =
            OptionBool(false).from_string(config.get_value("createrepo", "enabled")) ? true : false;
        if (createrepo_enabled) {
            if (need_rebuild_gpgcheck) {
                run_createrepo(repodir);
            }
            if (need_rebuild_nogpgcheck) {
                run_createrepo(repodir_nogpgcheck);
            }
        }
    }

private:
    static libdnf5::repo::RepoWeakPtr setup_local_repo(
        libdnf5::repo::RepoSackWeakPtr & repo_sack,
        const std::string & id,
        const std::string & name,
        const std::filesystem::path & basedir);

    void run_createrepo(const std::filesystem::path & dir);

    libdnf5::ConfigParser & config;
    std::filesystem::path repodir;
    std::filesystem::path repodir_nogpgcheck;
};


libdnf5::repo::RepoWeakPtr LocalPlugin::setup_local_repo(
    libdnf5::repo::RepoSackWeakPtr & repo_sack,
    const std::string & id,
    const std::string & name,
    const std::filesystem::path & basedir) {
    auto repo = repo_sack->create_repo(id);
    repo->get_config().get_name_option().set(name);
    repo->get_config().get_baseurl_option().set("file://" + std::filesystem::absolute(basedir).string());
    repo->get_config().get_skip_if_unavailable_option().set(true);
    // Make local repo preferred over other repos (which by default have cost 1000)
    repo->get_config().get_cost_option().set(500);
    // The repo should never be cached:
    // - Users expect the packages to be available in it right after running a transaction
    //   but this transaction would create a cache for the repo.
    // - The repo is usually local
    repo->get_config().get_metadata_expire_option().set(0);
    return repo;
}


void LocalPlugin::run_createrepo(const std::filesystem::path & dir) {
    std::vector<const char *> c_args{"--update", dir.c_str()};

    // --quiet is on by default
    OptionBool quiet_option = OptionBool(true);
    OptionBool verbose_option = OptionBool(false);

    if (config.has_option("createrepo", "verbose")) {
        verbose_option.set(verbose_option.from_string(config.get_value("createrepo", "verbose")));
    }
    if (config.has_option("createrepo", "quiet")) {
        quiet_option.set(quiet_option.from_string(config.get_value("createrepo", "quiet")));
    }

    if (verbose_option.get_value()) {
        c_args.push_back("--verbose");
    } else if (quiet_option.get_value()) {
        c_args.push_back("--quiet");
    }

    std::filesystem::path cachedir;
    if (config.has_option("createrepo", "cachedir")) {
        cachedir = config.get_value("createrepo", "cachedir");
        if (!cachedir.empty()) {
            c_args.push_back("--cachedir");
            c_args.push_back(cachedir.c_str());
        }
    }
    // execvp expects null terminated array of arguments
    c_args.push_back(nullptr);

    // TODO(mblaha): use libdnf5::utils::subprocess::run() instead of manual fork/exec
    int pipefd[2];
    if (pipe2(pipefd, O_CLOEXEC) == -1) {
        throw SystemError(errno, M_("Local plugin: Cannot create pipe"));
    }

    pid_t pid = fork();
    if (pid == -1) {
        throw SystemError(errno, M_("Local plugin: Cannot fork"));
    }

    if (pid == 0) {
        // This is the child process
        // Redirect stdout and stderr to the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);

        // Run program
        execvp("createrepo_c", const_cast<char * const *>(c_args.data()));

        // execvp should replace this process if it doesn't there was an error
        printf("Failed to execvp createrepo_c: %s\n", strerror(errno));
        _exit(255);
    } else {
        // This is the parent process
        // Close unused write pipe end
        close(pipefd[1]);

        char buffer[1024];
        ssize_t n;
        std::string output;
        while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            output += buffer;
        }
        close(pipefd[0]);

        if (!output.empty()) {
            auto & base = get_base();
            auto & logger = *base.get_logger();

            if (output.back() == '\n') {
                output.pop_back();
            }

            for (const auto & line : libdnf5::utils::string::split(output, "\n")) {
                logger.info("local plugin: createrepo_c: {}", line);
                // This print can interleave with the installation transaction progressbar
                // but I think we are missing an API the could print this correctly.
                printf("local plugin: createrepo_c: %s\n", line.c_str());
            }
        }

        // Wait for child
        int child_exit_status;
        int rc = waitpid(pid, &child_exit_status, 0);
        if (rc == -1) {
            throw SystemError(errno, M_("Local plugin: Cannot waitpid"));
        }

        if (WIFEXITED(child_exit_status)) {
            // Terminated normally (exit, _exit, returning from main) -> check exit code
            if (const int exit_status = WEXITSTATUS(child_exit_status); exit_status != 0) {
                throw LocalPluginError(M_("Createrepo_c process exited with code {}"), exit_status);
            }
        } else if (WIFSIGNALED(child_exit_status)) {
            throw LocalPluginError(M_("Createrepo_c process killed by signal {}"), WTERMSIG(child_exit_status));
        }
    }
}


std::exception_ptr last_exception;

}  // namespace


PluginAPIVersion libdnf_plugin_get_api_version(void) {
    return REQUIRED_PLUGIN_API_VERSION;
}

const char * libdnf_plugin_get_name(void) {
    return PLUGIN_NAME;
}

plugin::Version libdnf_plugin_get_version(void) {
    return PLUGIN_VERSION;
}

plugin::IPlugin * libdnf_plugin_new_instance(
    [[maybe_unused]] LibraryVersion library_version,
    libdnf5::plugin::IPluginData & data,
    libdnf5::ConfigParser & parser) try {
    return new LocalPlugin(data, parser);
} catch (...) {
    last_exception = std::current_exception();
    return nullptr;
}

void libdnf_plugin_delete_instance(plugin::IPlugin * plugin_object) {
    delete plugin_object;
}

std::exception_ptr * libdnf_plugin_get_last_exception(void) {
    return &last_exception;
}
