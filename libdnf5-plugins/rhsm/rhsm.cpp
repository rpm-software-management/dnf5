/*
Copyright Contributors to the dnf5 project.

This file is part of dnf5: https://github.com/rpm-software-management/dnf5/

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

#include <fcntl.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/common/exception.hpp>
#include <libdnf5/plugin/iplugin.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <rhsm/rhsm.h>
#include <unistd.h>


using namespace libdnf5;

namespace {

constexpr const char * PLUGIN_NAME = "rhsm";
constexpr plugin::Version PLUGIN_VERSION{0, 1, 0};
constexpr PluginAPIVersion REQUIRED_PLUGIN_API_VERSION{.major = 2, .minor = 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{
    "Jaroslav Rohel", "jrohel@redhat.com", "RHSM (Red Hat Subscription Manager) Plugin."};


class Rhsm : public plugin::IPlugin {
public:
    Rhsm(libdnf5::plugin::IPluginData & data, libdnf5::ConfigParser &) : IPlugin(data) {}
    virtual ~Rhsm() = default;

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

    void post_base_setup() override { setup_enrollments(); }

private:
    void setup_enrollments();
};


class RhsmPluginError : public libdnf5::Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::plugin"; }
    const char * get_name() const noexcept override { return "RhsmPluginError"; }
};


// Resyncs the enrollment with the vendor system. This can change the contents
// of the repositories configuration files according to the subscription levels.
void Rhsm::setup_enrollments() {
    const auto & config = get_base().get_config();

    // All of the subman stuff only works as root, if we're not
    // root, assume we're running in the test suite, or we're part of
    // e.g. rpm-ostree which is trying to run totally as non-root.
    if (getuid() != 0) {
        return;
    }

    // If /var/lib/rhsm exists, then we can assume that
    // subscription-manager is installed, and thus don't need to
    // manage redhat.repo via librhsm.
    if (g_file_test("/var/lib/rhsm", G_FILE_TEST_EXISTS)) {
        return;
    }

    const auto & repo_dirs = config.get_reposdir_option().get_value();
    if (repo_dirs.empty()) {
        throw RhsmPluginError(M_("Missing path to repository configuration directory"));
    }
    g_autofree gchar * repofname = g_build_filename(repo_dirs[0].c_str(), "redhat.repo", NULL);
    g_autoptr(RHSMContext) rhsm_ctx = rhsm_context_new();
    g_autoptr(GKeyFile) repofile = rhsm_utils_yum_repo_from_context(rhsm_ctx);

    bool same_content = false;
    do {
        int fd;
        if ((fd = open(repofname, O_RDONLY | O_CLOEXEC)) == -1) {
            break;
        }
        gsize length;
        g_autofree gchar * data = g_key_file_to_data(repofile, &length, NULL);
        auto file_len = lseek(fd, 0, SEEK_END);
        if (file_len != static_cast<off_t>(length)) {
            close(fd);
            break;
        }
        if (length > 0) {
            g_autofree gchar * buf = g_new(gchar, length);
            lseek(fd, 0, SEEK_SET);
            auto readed = read(fd, buf, length);
            close(fd);
            if (readed != file_len || memcmp(buf, data, length) != 0) {
                break;
            }
        } else {
            close(fd);
        }
        same_content = true;
    } while (false);

    if (!same_content) {
        g_autoptr(GError) err = NULL;
        if (!g_key_file_save_to_file(repofile, repofname, &err)) {
            throw RhsmPluginError(
                M_("Cannot save repository configuration to file \"{}\": {}"),
                std::string(repofname),
                std::string(err->message));
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
    return new Rhsm(data, parser);
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
