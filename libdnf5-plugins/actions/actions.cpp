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

#include <fmt/format.h>
#include <json.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/base/transaction.hpp>
#include <libdnf5/common/exception.hpp>
#include <libdnf5/common/sack/match_string.hpp>
#include <libdnf5/plugin/iplugin.hpp>
#include <libdnf5/repo/repo_errors.hpp>
#include <libdnf5/repo/repo_query.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/patterns.hpp>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace libdnf5;

namespace {

constexpr const char * PLUGIN_NAME = "actions";
constexpr plugin::Version PLUGIN_VERSION{1, 2, 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{"Jaroslav Rohel", "jrohel@redhat.com", "Actions Plugin."};

// Represents one line in action file
struct Action {
    std::filesystem::path file_path;
    int line_number;
    std::string pkg_filter;
    enum class Direction { IN, OUT, ALL } direction;
    std::string command;
    std::vector<std::string> args;
    enum class Mode { PLAIN, JSON } mode;
};


// Represents one command to run
struct CommandToRun {
    [[nodiscard]] bool operator<(const CommandToRun & other) const noexcept;
    const Action & action;
    std::string command;
    std::vector<std::string> args;
};


// Enum of supported hooks
enum class Hooks {
    PRE_BASE_SETUP,
    POST_BASE_SETUP,
    REPOS_CONFIGURED,
    REPOS_LOADED,
    PRE_ADD_CMDLINE_PACKAGES,
    POST_ADD_CMDLINE_PACKAGES,
    PRE_TRANS,
    POST_TRANS
};


class Actions final : public plugin::IPlugin {
public:
    Actions(libdnf5::plugin::IPluginData & data, libdnf5::ConfigParser &) : IPlugin(data) {}
    virtual ~Actions() = default;

    PluginAPIVersion get_api_version() const noexcept override { return PLUGIN_API_VERSION; }

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

    void init() override { parse_action_files(); }

    void pre_base_setup() override {
        current_hook = Hooks::PRE_BASE_SETUP;
        on_hook(pre_base_setup_actions);
    }

    void post_base_setup() override {
        current_hook = Hooks::POST_BASE_SETUP;
        on_hook(post_base_setup_actions);
    }

    void repos_configured() override {
        current_hook = Hooks::REPOS_CONFIGURED;
        on_hook(repos_configured_actions);
    }

    void repos_loaded() override {
        current_hook = Hooks::REPOS_LOADED;
        on_hook(repos_loaded_actions);
    }

    void pre_add_cmdline_packages(const std::vector<std::string> & paths) override {
        current_hook = Hooks::PRE_ADD_CMDLINE_PACKAGES;
        this->cmdline_packages_paths = &paths;
        on_hook(pre_add_cmdline_packages_actions);
    }

    void post_add_cmdline_packages() override {
        current_hook = Hooks::POST_ADD_CMDLINE_PACKAGES;
        this->cmdline_packages_paths = nullptr;
        on_hook(post_add_cmdline_packages_actions);
    }

    void pre_transaction(const libdnf5::base::Transaction & transaction) override {
        current_hook = Hooks::PRE_TRANS;
        on_transaction(transaction, pre_trans_actions);
    }

    void post_transaction(const libdnf5::base::Transaction & transaction) override {
        current_hook = Hooks::POST_TRANS;
        on_transaction(transaction, post_trans_actions);
    }

private:
    void parse_action_files();
    void on_hook(const std::vector<Action> & actions);
    void on_transaction(const libdnf5::base::Transaction & transaction, const std::vector<Action> & actions);
    void execute_command(CommandToRun & command);

    [[nodiscard]] std::pair<std::string, bool> substitute(
        const libdnf5::base::TransactionPackage * trans_pkg,
        const libdnf5::rpm::Package * pkg,
        std::string_view in,
        std::filesystem::path file,
        int line_number);

    [[nodiscard]] std::pair<std::vector<std::string>, bool> substitute_args(
        const libdnf5::base::TransactionPackage * trans_pkg, const libdnf5::rpm::Package * pkg, const Action & action);

    std::vector<std::pair<std::string, std::string>> get_conf(const std::string & key);
    std::vector<std::pair<std::string, std::string>> set_conf(const std::string & key, const std::string & value);

    void process_plain_communication(const CommandToRun & command, int in_fd);
    void process_command_output_line(const CommandToRun & command, std::string_view line);

    void process_json_communication(const CommandToRun & command, int in_fd, int out_fd);
    void process_json_command(const CommandToRun & command, struct json_object * request, int out_fd);

    // Parsed actions for individual hooks
    std::vector<Action> pre_base_setup_actions;
    std::vector<Action> post_base_setup_actions;
    std::vector<Action> repos_configured_actions;
    std::vector<Action> repos_loaded_actions;
    std::vector<Action> pre_add_cmdline_packages_actions;
    std::vector<Action> post_add_cmdline_packages_actions;
    std::vector<Action> pre_trans_actions;
    std::vector<Action> post_trans_actions;

    // Currently serviced hook
    Hooks current_hook;

    // During `pre_add_cmdline_packages` hook it points to paths to commandline packages. Otherwise it is nullptr.
    const std::vector<std::string> * cmdline_packages_paths = nullptr;

    // cache for sharing between pre_transaction and post_transaction hooks
    bool transaction_cached = false;
    std::vector<libdnf5::base::TransactionPackage> trans_packages;
    std::map<libdnf5::rpm::PackageId, const libdnf5::base::TransactionPackage *> pkg_id_to_trans_pkg;
    std::optional<libdnf5::rpm::PackageQuery> in_full_query;
    std::optional<libdnf5::rpm::PackageQuery> out_full_query;
    std::optional<libdnf5::rpm::PackageQuery> all_full_query;

    // store temporary variables for sharing data between actions (executables)
    std::map<std::string, std::string> tmp_variables;
};


class ActionsPluginError : public libdnf5::Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::plugin"; }
    const char * get_name() const noexcept override { return "ActionsPluginError"; }
};


// The ConfigError exception is handled internally. It will not leave the actions plugin.
class ConfigError : public std::runtime_error {
    using runtime_error::runtime_error;
};


bool CommandToRun::operator<(const CommandToRun & other) const noexcept {
    if (command == other.command) {
        if (args.size() == other.args.size()) {
            for (size_t i = 0; i < args.size(); ++i) {
                if (args[i] != other.args[i]) {
                    return args[i] < other.args[i];
                }
            }
        }
        return args.size() < other.args.size();
    }
    return command < other.command;
}

// splits the string into a vector, delimiter is space, spaces escaped by \ are ignored
std::vector<std::string> split(const std::string & str) {
    std::vector<std::string> ret;

    bool escape = false;
    auto it = str.begin();
    while (true) {
        while (*it == ' ') {
            ++it;
        }
        if (*it == '\0' || *it == '\n' || *it == '\r') {
            break;
        }
        auto it_start = it;
        for (; (escape || *it != ' ') && *it != '\0' && *it != '\n' && *it != '\r'; ++it) {
            escape = !escape && *it == '\\';
        }
        ret.emplace_back(it_start, it);
    }

    return ret;
}


// Replaces ',' with the escape sequence "\\x2C". (one '\' is removed later)
std::string escape_list_value(const std::string & value) {
    std::size_t escaped_chars = 0;
    for (const char ch : value) {
        if (ch == ',') {
            escaped_chars += 4;
        }
    }
    if (escaped_chars == 0) {
        return value;
    }
    std::string ret;
    ret.reserve(value.length() + escaped_chars);
    for (const char ch : value) {
        if (ch == ',') {
            ret += "\\\\x2C";
        } else {
            ret += ch;
        }
    }
    return ret;
}


std::pair<std::string, bool> Actions::substitute(
    const libdnf5::base::TransactionPackage * trans_pkg,
    const libdnf5::rpm::Package * pkg,
    std::string_view in,
    std::filesystem::path file,
    int line_number) {
    auto & base = get_base();
    auto & logger = *base.get_logger();
    std::string ret;
    bool error = false;
    size_t pos = 0;
    do {
        auto var_pos = in.find("${", pos);
        ret += in.substr(pos, var_pos - pos);
        if (var_pos == std::string::npos) {
            break;
        }
        var_pos += 2;
        auto var_end_pos = in.find('}', var_pos);
        if (var_end_pos == std::string::npos) {
            logger.error(
                "Actions plugin: Syntax error: Incomplete variable name \"{}\" in file \"{}\" on line {}",
                std::string(in.substr(var_pos - 2)),
                file.native(),
                line_number);
            error = true;
            break;
        }

        const auto var_name = in.substr(var_pos, var_end_pos - var_pos);
        std::optional<std::string> var_value;
        if (var_name == "pid") {
            var_value = std::to_string(getpid());
        } else if (var_name.starts_with("plugin.")) {
            auto plugin_key = var_name.substr(7);
            if (plugin_key == "version") {
                var_value = fmt::format("{}.{}.{}", PLUGIN_VERSION.major, PLUGIN_VERSION.minor, PLUGIN_VERSION.micro);
            }
        } else if (var_name.starts_with("conf.")) {
            auto key = std::string(var_name.substr(5));
            const auto equal_pos = key.find('=');
            const auto dot_pos = key.find('.');
            if (dot_pos < equal_pos) {
                // It is a repository option. The repoid is part of the key and can contain globs.
                // Will be substituted by a list of "repoid.option=value" pairs for the matching repositories.
                // Pairs are separated by ',' character. The ',' character in the value is replaced by escape sequence.
                // Supported formats: `<repoid_pattern>.<opt_name>` or `<repoid_pattern>.<opt_name>=<value_pattern>`
                std::string value_pattern;
                if (equal_pos != std::string::npos) {
                    value_pattern = key.substr(equal_pos + 1);
                    key = key.substr(0, equal_pos);
                }
                try {
                    const bool is_glob_pattern = utils::is_glob_pattern(value_pattern.c_str());
                    const auto list_key_vals = get_conf(key);
                    for (const auto & [key, val] : list_key_vals) {
                        if (!value_pattern.empty()) {
                            if (is_glob_pattern) {
                                if (!sack::match_string(val, sack::QueryCmp::GLOB, value_pattern)) {
                                    continue;
                                }
                            } else {
                                if (val != value_pattern) {
                                    continue;
                                }
                            }
                        }
                        if (var_value) {
                            *var_value += "," + key + '=' + escape_list_value(val);
                        } else {
                            var_value = key + '=' + escape_list_value(val);
                        }
                    }
                } catch (const ConfigError & ex) {
                }
            } else {
                auto config_opts = base.get_config().opt_binds();
                auto it = config_opts.find(key);
                if (it != config_opts.end()) {
                    var_value = it->second.get_value_string();
                }
            }
        } else if (var_name.starts_with("var.")) {
            auto vars = base.get_vars();
            try {
                var_value = vars->get_value(std::string(var_name.substr(4)));
            } catch (std::out_of_range &) {
            }
        } else if (var_name.starts_with("tmp.")) {
            if (auto it = tmp_variables.find(std::string(var_name.substr(4))); it != tmp_variables.end()) {
                var_value = it->second;
            }
        } else if (var_name.starts_with("pkg.")) {
            auto pkg_key = var_name.substr(4);
            if (pkg) {
                if (pkg_key == "name") {
                    var_value = pkg->get_name();
                } else if (pkg_key == "arch") {
                    var_value = pkg->get_arch();
                } else if (pkg_key == "version") {
                    var_value = pkg->get_version();
                } else if (pkg_key == "release") {
                    var_value = pkg->get_release();
                } else if (pkg_key == "epoch") {
                    var_value = pkg->get_epoch();
                } else if (pkg_key == "na") {
                    var_value = pkg->get_na();
                } else if (pkg_key == "evr") {
                    var_value = pkg->get_evr();
                } else if (pkg_key == "nevra") {
                    var_value = pkg->get_nevra();
                } else if (pkg_key == "full_nevra") {
                    var_value = pkg->get_full_nevra();
                } else if (pkg_key == "repo_id") {
                    var_value = pkg->get_repo_id();
                } else if (pkg_key == "license") {
                    var_value = pkg->get_license();
                } else if (pkg_key == "location") {
                    var_value = pkg->get_location();
                } else if (pkg_key == "vendor") {
                    var_value = pkg->get_vendor();
                }
            }
            if (trans_pkg) {
                if (pkg_key == "action") {
                    var_value = libdnf5::transaction::transaction_item_action_to_letter(trans_pkg->get_action());
                }
            }
        }
        if (var_value) {
            ret += *var_value;
        } else {
            logger.error(
                "Actions plugin: Unknown variable \"{}\" in file \"{}\" on line {}",
                std::string(var_name),
                file.native(),
                line_number);
            error = true;
            break;
        }

        pos = var_end_pos + 1;
    } while (pos < in.size());
    return {ret, error};
}

std::pair<std::vector<std::string>, bool> Actions::substitute_args(
    const libdnf5::base::TransactionPackage * trans_pkg, const libdnf5::rpm::Package * pkg, const Action & action) {
    std::vector<std::string> substituted_args;
    substituted_args.reserve(action.args.size());
    for (const auto & arg : action.args) {
        auto [value, subst_error] = substitute(trans_pkg, pkg, arg, action.file_path, action.line_number);
        if (subst_error) {
            return {substituted_args, true};
        }
        substituted_args.emplace_back(value);
    }
    return {substituted_args, false};
}

void unescape(std::string & str) {
    bool escape = false;
    size_t dst_pos = 0;
    for (size_t src_pos = 0; src_pos < str.size(); ++src_pos) {
        if (escape) {
            switch (str[src_pos]) {
                case 'a':
                    str[dst_pos++] = '\a';
                    break;
                case 'b':
                    str[dst_pos++] = '\b';
                    break;
                case 'f':
                    str[dst_pos++] = '\f';
                    break;
                case 'n':
                    str[dst_pos++] = '\n';
                    break;
                case 'r':
                    str[dst_pos++] = '\r';
                    break;
                case 't':
                    str[dst_pos++] = '\t';
                    break;
                case 'v':
                    str[dst_pos++] = '\v';
                    break;
                default:
                    str[dst_pos++] = str[src_pos];
            }
            escape = false;
        } else if (str[src_pos] == '\\') {
            escape = true;
        } else {
            str[dst_pos++] = str[src_pos];
        }
    }
    str.resize(dst_pos);
}

void Actions::on_hook(const std::vector<Action> & actions) {
    if (actions.empty()) {
        return;
    }

    std::set<CommandToRun> unique_commands_to_run;  // std::set is used to detect duplicate commands

    for (const auto & action : actions) {
        if (auto [substituted_args, subst_error] = substitute_args(nullptr, nullptr, action); !subst_error) {
            for (auto & arg : substituted_args) {
                unescape(arg);
            }
            CommandToRun cmd_to_run{action, action.command, std::move(substituted_args)};
            if (auto [it, inserted] = unique_commands_to_run.insert(cmd_to_run); inserted) {
                execute_command(cmd_to_run);
            }
        }
    }
}

void Actions::parse_action_files() {
    const auto & config = get_base().get_config();
    const char * env_plugins_config_dir = std::getenv("LIBDNF_PLUGINS_CONFIG_DIR");
    const std::string plugins_config_dir = env_plugins_config_dir && config.get_pluginconfpath_option().get_priority() <
                                                                         libdnf5::Option::Priority::COMMANDLINE
                                               ? env_plugins_config_dir
                                               : config.get_pluginconfpath_option().get_value();

    auto action_dir_path = std::filesystem::path(plugins_config_dir) / "actions.d";
    std::vector<std::filesystem::path> action_paths;
    std::error_code ec;  // Do not report errors if config_dir_path refers to a non-existing file or not a directory
    for (const auto & p : std::filesystem::directory_iterator(action_dir_path, ec)) {
        if ((p.is_regular_file() || p.is_symlink()) && p.path().extension() == ".actions") {
            action_paths.emplace_back(p.path());
        }
    }
    std::sort(action_paths.begin(), action_paths.end());

    for (const auto & path : action_paths) {
        std::ifstream action_file(path);
        std::string line;
        int line_number = 0;
        while (std::getline(action_file, line)) {
            ++line_number;
            if (line.empty() || line[0] == '#') {
                continue;
            }
            auto pkg_filter_pos = line.find(':');
            if (pkg_filter_pos == std::string::npos) {
                throw ActionsPluginError(
                    M_("Error in file \"{}\" on line {}: \"HOOK:PKG_FILTER:DIRECTION:OPTIONS:CMD\" format expected"),
                    path.native(),
                    line_number);
            }
            ++pkg_filter_pos;
            auto direction_pos = line.find(':', pkg_filter_pos);
            if (direction_pos == std::string::npos) {
                throw ActionsPluginError(
                    M_("Error in file \"{}\" on line {}: \"HOOK:PKG_FILTER:DIRECTION:OPTIONS:CMD\" format expected"),
                    path.native(),
                    line_number);
            }
            ++direction_pos;
            auto options_pos = line.find(':', direction_pos);
            if (options_pos == std::string::npos) {
                throw ActionsPluginError(
                    M_("Error in file \"{}\" on line {}: \"HOOK:PKG_FILTER:DIRECTION:OPTIONS:CMD\" format expected"),
                    path.native(),
                    line_number);
            }
            ++options_pos;
            auto command_pos = line.find(':', options_pos);
            if (command_pos == std::string::npos) {
                throw ActionsPluginError(
                    M_("Error in file \"{}\" on line {}: \"HOOK:PKG_FILTER:DIRECTION:OPTIONS:CMD\" format expected"),
                    path.native(),
                    line_number);
            }
            ++command_pos;

            bool action_enabled{true};
            std::string mode = "plain";
            auto options_str = line.substr(options_pos, command_pos - options_pos - 1);
            const auto options = split(options_str);
            for (const auto & opt : options) {
                if (opt.starts_with("enabled=")) {
                    const auto value = opt.substr(8);
                    auto installroot_path = config.get_installroot_option().get_value();
                    bool installroot = installroot_path != "/";
                    if (value == "1") {
                        action_enabled = true;
                    } else if (value == "host-only") {
                        action_enabled = !installroot;
                    } else if (value == "installroot-only") {
                        action_enabled = installroot;
                    } else {
                        throw ActionsPluginError(
                            M_("Error in file \"{}\" on line {}: Unknown \"enabled\" option value \"{}\""),
                            path.native(),
                            line_number,
                            value);
                    }
                } else if (opt.starts_with("mode=")) {
                    mode = opt.substr(5);
                } else {
                    throw ActionsPluginError(
                        M_("Error in file \"{}\" on line {}: Unknown option \"{}\""), path.native(), line_number, opt);
                }
            }
            if (!action_enabled) {
                continue;
            }

            Hooks hook;
            if (line.starts_with("pre_base_setup:")) {
                hook = Hooks::PRE_BASE_SETUP;
            } else if (line.starts_with("post_base_setup:")) {
                hook = Hooks::POST_BASE_SETUP;
            } else if (line.starts_with("repos_configured:")) {
                hook = Hooks::REPOS_CONFIGURED;
            } else if (line.starts_with("repos_loaded:")) {
                hook = Hooks::REPOS_LOADED;
            } else if (line.starts_with("pre_add_cmdline_packages:")) {
                hook = Hooks::PRE_ADD_CMDLINE_PACKAGES;
            } else if (line.starts_with("post_add_cmdline_packages:")) {
                hook = Hooks::POST_ADD_CMDLINE_PACKAGES;
            } else if (line.starts_with("pre_transaction:")) {
                hook = Hooks::PRE_TRANS;
            } else if (line.starts_with("post_transaction:")) {
                hook = Hooks::POST_TRANS;
            } else {
                throw ActionsPluginError(
                    M_("Error in file \"{}\" on line {}: Unknown hook \"{}\""),
                    path.native(),
                    line_number,
                    line.substr(0, pkg_filter_pos - 1));
            }

            auto pkg_filter = line.substr(pkg_filter_pos, direction_pos - pkg_filter_pos - 1);
            if (hook != Hooks::PRE_TRANS && hook != Hooks::POST_TRANS) {
                if (!pkg_filter.empty()) {
                    throw ActionsPluginError(
                        M_("Error in file \"{}\" on line {}: Package filter can only be used in PRE_TRANS and "
                           "POST_TRANS hooks"),
                        path.native(),
                        line_number);
                }
            }

            auto direction = line.substr(direction_pos, options_pos - direction_pos - 1);
            if (pkg_filter.empty() && !direction.empty()) {
                throw ActionsPluginError(
                    M_("Error in file \"{}\" on line {}: Cannot use direction without package filter"),
                    path.native(),
                    line_number);
            }

            Action act;
            act.file_path = path;
            act.line_number = line_number;
            act.pkg_filter = pkg_filter;
            if (direction == "in") {
                act.direction = Action::Direction::IN;
            } else if (direction == "out") {
                act.direction = Action::Direction::OUT;
            } else if (direction == "") {
                act.direction = Action::Direction::ALL;
            } else {
                throw ActionsPluginError(
                    M_("Error in file \"{}\" on line {}: Unknown package direction \"{}\""),
                    path.native(),
                    line_number,
                    direction);
            }
            if (mode == "plain") {
                act.mode = Action::Mode::PLAIN;
            } else if (mode == "json") {
                act.mode = Action::Mode::JSON;
            } else {
                throw ActionsPluginError(
                    M_("Error in file \"{}\" on line {}: Unknown mode \"{}\""), path.native(), line_number, mode);
            }

            act.args = split(line.substr(command_pos));
            if (act.args.empty()) {
                throw ActionsPluginError(
                    M_("Error in file \"{}\" on line {}: Missing command"), path.native(), line_number);
            }
            act.command = act.args[0];

            switch (hook) {
                case Hooks::PRE_BASE_SETUP:
                    pre_base_setup_actions.emplace_back(std::move(act));
                    break;
                case Hooks::POST_BASE_SETUP:
                    post_base_setup_actions.emplace_back(std::move(act));
                    break;
                case Hooks::REPOS_CONFIGURED:
                    repos_configured_actions.emplace_back(std::move(act));
                    break;
                case Hooks::REPOS_LOADED:
                    repos_loaded_actions.emplace_back(std::move(act));
                    break;
                case Hooks::PRE_ADD_CMDLINE_PACKAGES:
                    pre_add_cmdline_packages_actions.emplace_back(std::move(act));
                    break;
                case Hooks::POST_ADD_CMDLINE_PACKAGES:
                    post_add_cmdline_packages_actions.emplace_back(std::move(act));
                    break;
                case Hooks::PRE_TRANS:
                    pre_trans_actions.emplace_back(std::move(act));
                    break;
                case Hooks::POST_TRANS:
                    post_trans_actions.emplace_back(std::move(act));
            }
        }
    }
}


// Parses the input key and returns the repoid and option name.
// If there is a global option on the input, repoid will be an empty string
std::pair<std::string, std::string> get_repoid_and_optname_from_key(std::string_view key) {
    std::string repo_id;
    std::string opt_name;

    auto dot_pos = key.rfind('.');
    if (dot_pos != std::string::npos) {
        if (dot_pos == key.size() - 1) {
            throw ConfigError(fmt::format("Badly formatted argument value: Last key character cannot be '.': {}", key));
        }
        repo_id = key.substr(0, dot_pos);
        opt_name = key.substr(dot_pos + 1);
    } else {
        opt_name = key;
    }

    return {repo_id, opt_name};
}


// Returns a list of matching "key=value" pairs.
// The key can be a global option or repository option. The input key can contain globs in repository name.
std::vector<std::pair<std::string, std::string>> Actions::get_conf(const std::string & key) {
    auto & base = get_base();
    std::vector<std::pair<std::string, std::string>> list_set_key_vals;

    auto [repo_id_pattern, opt_name] = get_repoid_and_optname_from_key(key);
    if (!repo_id_pattern.empty()) {
        repo::RepoQuery query(base);
        query.filter_id(repo_id_pattern, sack::QueryCmp::GLOB);
        for (auto repo : query) {
            auto config_opts = repo->get_config().opt_binds();
            auto it = config_opts.find(opt_name);
            if (it == config_opts.end()) {
                throw ConfigError(fmt::format("Unknown repo config option: {}", key));
            }
            std::string value;
            try {
                value = it->second.get_value_string();
            } catch (libdnf5::OptionError & ex) {
                throw ConfigError(fmt::format("Cannot get repo config option \"{}\": {}", key, ex.what()));
            }
            list_set_key_vals.emplace_back(repo->get_id() + '.' + it->first, value);
        }
    } else {
        auto config_opts = base.get_config().opt_binds();
        auto it = config_opts.find(key);
        if (it == config_opts.end()) {
            throw ConfigError(fmt::format("Unknown config option \"{}\"", key));
        }
        std::string value;
        try {
            value = it->second.get_value_string();
        } catch (libdnf5::OptionError & ex) {
            throw ConfigError(fmt::format("Cannot get config option \"{}\": {}", key, ex.what()));
        }
        list_set_key_vals.emplace_back(key, value);
    }
    return list_set_key_vals;
}


// Sets the matching keys to the given value.
// Returns a list of matching "key=value" pairs. The key can be a global option or repository option.
// The input key can contain globs in repository name. New value is returned.
std::vector<std::pair<std::string, std::string>> Actions::set_conf(const std::string & key, const std::string & value) {
    auto & base = get_base();
    std::vector<std::pair<std::string, std::string>> list_set_key_vals;

    auto [repo_id_pattern, opt_name] = get_repoid_and_optname_from_key(key);
    if (!repo_id_pattern.empty()) {
        repo::RepoQuery query(base);
        query.filter_id(repo_id_pattern, sack::QueryCmp::GLOB);
        for (auto repo : query) {
            auto config_opts = repo->get_config().opt_binds();
            auto it = config_opts.find(opt_name);
            if (it == config_opts.end()) {
                throw ConfigError(fmt::format("Unknown repo config option: {}", key));
            }
            try {
                it->second.new_string(libdnf5::Option::Priority::PLUGINCONFIG, value);
            } catch (libdnf5::OptionError & ex) {
                throw ConfigError(fmt::format("Cannot set repo config option \"{}={}\": {}", key, value, ex.what()));
            }
            list_set_key_vals.emplace_back(repo->get_id() + '.' + it->first, value);
        }
    } else {
        auto config_opts = base.get_config().opt_binds();
        auto it = config_opts.find(key);
        if (it == config_opts.end()) {
            throw ConfigError(fmt::format("Unknown config option \"{}\"", key));
        }
        try {
            it->second.new_string(libdnf5::Option::Priority::PLUGINCONFIG, value);
        } catch (libdnf5::OptionError & ex) {
            throw ConfigError(fmt::format("Cannot set config option \"{}={}\": {}", key, value, ex.what()));
        }
        list_set_key_vals.emplace_back(key, value);
    }
    return list_set_key_vals;
}


void Actions::process_plain_communication(const CommandToRun & command, int in_fd) {
    char read_buf[256];
    std::string input;
    std::size_t num_tested_chars = 0;
    do {
        auto len = read(in_fd, read_buf, sizeof(read_buf));
        if (len > 0) {
            std::size_t line_begin_pos = 0;
            input.append(read_buf, static_cast<std::size_t>(len));
            std::string_view input_view(input);
            do {
                auto line_end_pos = input_view.find('\n', num_tested_chars);
                if (line_end_pos == std::string::npos) {
                    num_tested_chars = input_view.size();
                } else {
                    process_command_output_line(
                        command, input_view.substr(line_begin_pos, line_end_pos - line_begin_pos));
                    num_tested_chars = line_begin_pos = line_end_pos + 1;
                }
            } while (num_tested_chars < input_view.size());

            // shift - erase processed lines from the input buffer
            input.erase(0, line_begin_pos);
            num_tested_chars -= line_begin_pos;
            line_begin_pos = 0;
        } else {
            if (!input.empty()) {
                process_command_output_line(command, input);
            }
            break;
        }
    } while (true);
}


void Actions::process_command_output_line(const CommandToRun & command, std::string_view line) {
    auto & base = get_base();

    auto eq_pos = line.find('=');
    if (line.starts_with("tmp.")) {
        std::string var_name(line.substr(4, eq_pos - 4));
        if (eq_pos == std::string::npos) {
            tmp_variables.erase(var_name);
        } else {
            tmp_variables[var_name] = line.substr(eq_pos + 1);
        }
        return;
    }
    if (eq_pos == std::string::npos) {
        base.get_logger()->error(
            "Actions plugin: Syntax error from hook in file \"{}\" on line {}: Missing equal sign (=) in command "
            "output line \"{}\"",
            command.action.file_path.string(),
            command.action.line_number,
            std::string(line));
        return;
    }
    if (line.starts_with("conf.")) {
        std::string key(line.substr(5, eq_pos - 5));
        std::string conf_value(line.substr(eq_pos + 1));
        try {
            set_conf(key, conf_value);
        } catch (const ConfigError & ex) {
            base.get_logger()->error(
                "Actions plugin: Hook in file \"{}\" on line {}: {}",
                command.action.file_path.string(),
                command.action.line_number,
                ex.what());
        }
    } else if (line.starts_with("var.")) {
        std::string var_name(line.substr(4, eq_pos - 4));
        std::string var_value(line.substr(eq_pos + 1));
        base.get_vars()->set(var_name, var_value, libdnf5::Vars::Priority::PLUGIN);
    } else {
        base.get_logger()->error(
            "Actions plugin: Syntax error from hook in file \"{}\" on line {}: Command output line has to start with "
            "\"tmp.\" or \"conf.\" or \"var.\": \"{}\"",
            command.action.file_path.string(),
            command.action.line_number,
            std::string(line));
    }
}


class JsonRequestError : public std::runtime_error {
    using runtime_error::runtime_error;
};


class WriteError : public std::runtime_error {
    using runtime_error::runtime_error;
};


void Actions::process_json_communication(const CommandToRun & command, int in_fd, int out_fd) {
    auto & base = get_base();
    char read_buf[256];
    size_t read_offset = 0;
    bool first_read = true;
    auto * tok = json_tokener_new();
    std::unique_ptr<json_tokener, decltype(&json_tokener_free)> tok_owner(tok, &json_tokener_free);
    do {
        auto ret = read(in_fd, read_buf + read_offset, sizeof(read_buf) - read_offset);
        if (ret < 0) {
            auto err = std::strerror(errno);
            base.get_logger()->error(
                "Actions plugin: Error reading from pipe from hook in file \"{}\" on line {}: {}",
                command.action.file_path.string(),
                command.action.line_number,
                err);
            return;
        }
        auto len = static_cast<size_t>(ret) + read_offset;
        if (len > 0) {
            if (first_read) {
                size_t i = 0;
                while (i < len && read_buf[i] == '\n') {
                    ++i;
                }
                if (i == len) {
                    continue;
                }
                if (read_buf[i] != '{') {
                    base.get_logger()->error(
                        "Actions plugin: Syntax error in json request from hook in file \"{}\" on line {}: Missing "
                        "starting '{{' char",
                        command.action.file_path.string(),
                        command.action.line_number);
                    return;
                }
                if (i > 0) {
                    len -= i;
                    memmove(read_buf, read_buf + i, len);
                }
                first_read = false;
            }

            auto * jobj = json_tokener_parse_ex(tok, read_buf, static_cast<int>(len));
            if (jobj) {
                std::unique_ptr<json_object, decltype(&json_object_put)> jobj_owner(jobj, &json_object_put);

                auto parsed = json_tokener_get_parse_end(tok);
                read_offset = len - parsed;
                memmove(read_buf, read_buf + parsed, read_offset);
                json_tokener_reset(tok);
                first_read = true;

                try {
                    process_json_command(command, jobj, out_fd);
                } catch (const WriteError & ex) {
                    base.get_logger()->error(
                        "Actions plugin: Error in process request from hook in file \"{}\" on line {}: {}",
                        command.action.file_path.string(),
                        command.action.line_number,
                        ex.what());
                    return;
                }
            } else {
                auto jerr = json_tokener_get_error(tok);
                if (jerr != json_tokener_continue) {
                    base.get_logger()->error(
                        "Actions plugin: Syntax error in json request from hook in file \"{}\" on line {}: {}",
                        command.action.file_path.string(),
                        command.action.line_number,
                        json_tokener_error_desc(jerr));
                    return;
                }
            }
        } else {
            if (!first_read) {
                base.get_logger()->error(
                    "Actions plugin: Syntax error in json request from hook in file \"{}\" on line {}: Incomplete "
                    "input",
                    command.action.file_path.string(),
                    command.action.line_number);
            }
            return;
        }
    } while (true);
}


inline struct json_object * get_any_object_or_null(json_object * container, const char * key) {
    struct json_object * jobj;
    if (json_object_object_get_ex(container, key, &jobj)) {
        return jobj;
    }
    return nullptr;
}


inline struct json_object * get_any_object(json_object * container, const char * key) {
    struct json_object * jobj;
    if (!json_object_object_get_ex(container, key, &jobj)) {
        throw JsonRequestError(fmt::format("Key \"{}\" not found", key));
    }
    return jobj;
}


std::string_view get_string_view(json_object * jobj) {
    if (json_object_get_type(jobj) != json_type_string) {
        throw JsonRequestError("Bad json type");
    }
    return json_object_get_string(jobj);
}


std::string_view get_string_view(json_object * container, const char * key) {
    struct json_object * jobj = get_any_object(container, key);
    if (json_object_get_type(jobj) != json_type_string) {
        throw JsonRequestError(fmt::format("Bad json type of \"{}\" key", key));
    }
    return json_object_get_string(jobj);
}


json_object * get_object(json_object * container, const char * key) {
    struct json_object * jobj = get_any_object(container, key);
    if (json_object_get_type(jobj) != json_type_object) {
        throw JsonRequestError(fmt::format("Bad json type of \"{}\" key", key));
    }
    return jobj;
}


json_object * get_array(json_object * container, const char * key) {
    struct json_object * jobj = get_any_object(container, key);
    if (json_object_get_type(jobj) != json_type_array) {
        throw JsonRequestError(fmt::format("Bad json type of \"{}\" key", key));
    }
    return jobj;
}


[[__nodiscard__]] json_object * new_key_val_obj(
    const char * key_id, const char * key_val, const char * val_id, const char * val) {
    auto * jobj_key_val = json_object_new_object();
    json_object_object_add_ex(jobj_key_val, key_id, json_object_new_string(key_val), JSON_C_OBJECT_ADD_CONSTANT_KEY);
    json_object_object_add_ex(jobj_key_val, val_id, json_object_new_string(val), JSON_C_OBJECT_ADD_CONSTANT_KEY);
    return jobj_key_val;
}


void write_buf(int out_fd, const char * buf, size_t length) {
    auto to_write = length;
    while (to_write > 0) {
        const auto written = write(out_fd, buf + (length - to_write), to_write);
        if (written < 0) {
            throw WriteError(fmt::format("Cannot write response: {}", std::strerror(errno)));
        }
        to_write -= static_cast<size_t>(written);
    }
}


void write_json_object(struct json_object * jobject, int out_fd) {
    size_t json_length;
    auto * result_json = json_object_to_json_string_length(jobject, JSON_C_TO_STRING_SPACED, &json_length);
    write_buf(out_fd, result_json, json_length);
    write_buf(out_fd, "\n", 1);
}


libdnf5::sack::QueryCmp cmp_operator_from_string(std::string_view str_operator) {
    libdnf5::sack::QueryCmp ret =
        str_operator.starts_with("NOT_") ? libdnf5::sack::QueryCmp::NOT : libdnf5::sack::QueryCmp{0};
    std::string_view str = ret == libdnf5::sack::QueryCmp::NOT ? str_operator.substr(4) : str_operator;
    if (str == "EQ") {
        return ret | libdnf5::sack::QueryCmp::EQ;
    }
    if (str == "IEQ") {
        return ret | libdnf5::sack::QueryCmp::IEXACT;
    }
    if (str_operator == "GT") {
        return ret | libdnf5::sack::QueryCmp::GT;
    }
    if (str_operator == "GTE") {
        return ret | libdnf5::sack::QueryCmp::GTE;
    }
    if (str_operator == "LT") {
        return ret | libdnf5::sack::QueryCmp::LT;
    }
    if (str_operator == "LTE") {
        return ret | libdnf5::sack::QueryCmp::LTE;
    }
    if (str_operator == "CONTAINS") {
        return ret | libdnf5::sack::QueryCmp::CONTAINS;
    }
    if (str_operator == "ICONTAINS") {
        return ret | libdnf5::sack::QueryCmp::ICONTAINS;
    }
    if (str_operator == "STARTSWITH") {
        return ret | libdnf5::sack::QueryCmp::STARTSWITH;
    }
    if (str_operator == "ISTARTSWITH") {
        return ret | libdnf5::sack::QueryCmp::ISTARTSWITH;
    }
    if (str_operator == "ENDSWITH") {
        return ret | libdnf5::sack::QueryCmp::ENDSWITH;
    }
    if (str_operator == "IENDSWITH") {
        return ret | libdnf5::sack::QueryCmp::IENDSWITH;
    }
    if (str_operator == "REGEX") {
        return ret | libdnf5::sack::QueryCmp::REGEX;
    }
    if (str_operator == "IREGEX") {
        return ret | libdnf5::sack::QueryCmp::IREGEX;
    }
    if (str_operator == "GLOB") {
        return ret | libdnf5::sack::QueryCmp::GLOB;
    }
    if (str_operator == "IGLOB") {
        return ret | libdnf5::sack::QueryCmp::IGLOB;
    }
    throw JsonRequestError(fmt::format("Bad compare operator \"{}\"", str_operator));
}


void Actions::process_json_command(const CommandToRun & command, struct json_object * request, int out_fd) {
    auto & base = get_base();
    auto logger = base.get_logger();

    if (auto type = json_object_get_type(request); type != json_type_object) {
        base.get_logger()->error("Actions plugin: Bad json type of command");
        return;
    }

    auto * jresult = json_object_new_object();
    std::unique_ptr<json_object, decltype(&json_object_put)> jresult_owner(jresult, &json_object_put);
    json_object_object_add_ex(jresult, "op", json_object_new_string("reply"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
    auto * jreturn = json_object_new_object();
    std::unique_ptr<json_object, decltype(&json_object_put)> jreturn_owner(jreturn, &json_object_put);

    try {
        auto op = get_string_view(request, "op");
        json_object_object_add_ex(
            jresult, "requested_op", json_object_new_string(op.data()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
        if (op == "get") {
            auto domain = get_string_view(request, "domain");
            json_object_object_add_ex(
                jresult, "domain", json_object_new_string(domain.data()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            if (domain == "conf") {
                auto * jargs = get_object(request, "args");
                auto key = std::string(get_string_view(jargs, "key"));
                try {
                    const auto list_set_key_vals = get_conf(key);
                    auto * jkeys_val = json_object_new_array();
                    json_object_object_add_ex(jreturn, "keys_val", jkeys_val, JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    for (auto & [set_key, set_value] : list_set_key_vals) {
                        auto * jset_key_val = new_key_val_obj("key", set_key.c_str(), "value", set_value.c_str());
                        json_object_array_add(jkeys_val, jset_key_val);
                    }
                    json_object_object_add_ex(
                        jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_object_add_ex(
                        jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                } catch (const ConfigError & ex) {
                    logger->error(
                        "Actions plugin: Hook in file \"{}\" on line {}: {}",
                        command.action.file_path.string(),
                        command.action.line_number,
                        ex.what());
                    json_object_object_add_ex(
                        jresult, "status", json_object_new_string("ERROR"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_object_add_ex(
                        jresult, "message", json_object_new_string(ex.what()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                }
            } else if (domain == "actions_attrs") {
                auto * jargs = get_object(request, "args");
                auto key_pattern = get_string_view(jargs, "key");
                auto * jactions_attrs = json_object_new_array();
                json_object_object_add_ex(jreturn, "actions_attrs", jactions_attrs, JSON_C_OBJECT_ADD_CONSTANT_KEY);

                for (const auto * const attr : {"pid", "version"}) {
                    if (sack::match_string(attr, sack::QueryCmp::GLOB, std::string(key_pattern))) {
                        if (std::string_view(attr) == "pid") {
                            auto * jactions_attr =
                                new_key_val_obj("key", attr, "value", std::to_string(getpid()).c_str());
                            json_object_array_add(jactions_attrs, jactions_attr);
                        } else if (std::string_view(attr) == "version") {
                            auto version = fmt::format(
                                "{}.{}.{}", PLUGIN_VERSION.major, PLUGIN_VERSION.minor, PLUGIN_VERSION.micro);
                            auto * jactions_attr = new_key_val_obj("key", attr, "value", version.c_str());
                            json_object_array_add(jactions_attrs, jactions_attr);
                        }
                    }
                }
                json_object_object_add_ex(jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                json_object_object_add_ex(
                    jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            } else if (domain == "vars") {
                auto * jargs = get_object(request, "args");
                auto name_pattern = std::string(get_string_view(jargs, "name"));
                auto * jnames_val = json_object_new_array();
                json_object_object_add_ex(jreturn, "vars", jnames_val, JSON_C_OBJECT_ADD_CONSTANT_KEY);
                for (const auto & [name, val_prio] : base.get_vars()->get_variables()) {
                    if (sack::match_string(name, sack::QueryCmp::GLOB, name_pattern)) {
                        auto * jname_val = new_key_val_obj("name", name.c_str(), "value", val_prio.value.c_str());
                        json_object_array_add(jnames_val, jname_val);
                    }
                }
                json_object_object_add_ex(jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                json_object_object_add_ex(
                    jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            } else if (domain == "actions_vars") {
                auto * jargs = get_object(request, "args");
                auto name_pattern = std::string(get_string_view(jargs, "name"));
                auto * jnames_val = json_object_new_array();
                json_object_object_add_ex(jreturn, "actions_vars", jnames_val, JSON_C_OBJECT_ADD_CONSTANT_KEY);
                for (const auto & [name, value] : tmp_variables) {
                    if (sack::match_string(name, sack::QueryCmp::GLOB, name_pattern)) {
                        auto * jname_val = new_key_val_obj("name", name.c_str(), "value", value.c_str());
                        json_object_array_add(jnames_val, jname_val);
                    }
                }
                json_object_object_add_ex(jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                json_object_object_add_ex(
                    jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            } else if (domain == "packages" || domain == "trans_packages") {
                if (domain == "trans_packages" && current_hook != Hooks::PRE_TRANS &&
                    current_hook != Hooks::POST_TRANS) {
                    throw JsonRequestError(
                        "Domain \"trans_packages\" used outside the hooks \"pre_transaction\" and "
                        "\"post_transaction\"");
                }
                auto * jargs = get_object(request, "args");

                const std::pair<
                    const char *,
                    std::function<std::string(const base::TransactionPackage *, const rpm::Package &)>>
                    attrs_list[] = {
                        {"direction",
                         [](const base::TransactionPackage * trans_pkg, const rpm::Package &) {
                             if (!trans_pkg) {
                                 return "";
                             }
                             return transaction_item_action_is_inbound(trans_pkg->get_action()) ? "IN" : "OUT";
                         }},
                        {"action",
                         [](const base::TransactionPackage * trans_pkg, const rpm::Package &) {
                             if (!trans_pkg) {
                                 return std::string{};
                             }
                             return transaction::transaction_item_action_to_letter(trans_pkg->get_action());
                         }},
                        {"name",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_name(); }},
                        {"arch",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_arch(); }},
                        {"version",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_version(); }},
                        {"release",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_release(); }},
                        {"epoch",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_epoch(); }},
                        {"na", [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_na(); }},
                        {"evr",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_evr(); }},
                        {"nevra",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_nevra(); }},
                        {"full_nevra",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) {
                             return pkg.get_full_nevra();
                         }},
                        {"download_size",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) {
                             return std::to_string(pkg.get_download_size());
                         }},
                        {"install_size",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) {
                             return std::to_string(pkg.get_install_size());
                         }},
                        {"repo_id",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_repo_id(); }},
                        {"license",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_license(); }},
                        {"location",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_location(); }},
                        {"vendor",
                         [](const base::TransactionPackage *, const rpm::Package & pkg) { return pkg.get_vendor(); }}};

                unsigned int requested_attrs = 0;
                auto * joutput = get_array(jargs, "output");
                for (size_t idx = 0; idx < json_object_array_length(joutput); ++idx) {
                    const auto requested_attr = get_string_view(json_object_array_get_idx(joutput, idx));
                    if (domain != "trans_packages") {
                        if (requested_attr == "direction") {
                            throw JsonRequestError(
                                "Requested output \"direction\" outside the domain \"trans_packages\"");

                        } else if (requested_attr == "action") {
                            throw JsonRequestError("Requested output \"action\" outside the domain \"trans_packages\"");
                        }
                    }
                    for (std::size_t idx = 0; idx < sizeof(attrs_list) / sizeof(attrs_list[0]); ++idx) {
                        if (attrs_list[idx].first == requested_attr) {
                            requested_attrs |= 1 << idx;
                        }
                    }
                }

                libdnf5::sack::ExcludeFlags query_flags{0};
                auto * jparams = get_any_object_or_null(jargs, "params");
                if (jparams) {
                    get_array(jargs, "params");  // check, must be array
                    for (size_t idx = 0; idx < json_object_array_length(jparams); ++idx) {
                        auto * jparam = json_object_array_get_idx(jparams, idx);
                        const auto param_key = get_string_view(jparam, "key");
                        if (param_key == "IGNORE_EXCLUDES") {
                            query_flags = query_flags | libdnf5::sack::ExcludeFlags::IGNORE_EXCLUDES;
                        } else if (param_key == "IGNORE_MODULAR_EXCLUDES") {
                            query_flags = query_flags | libdnf5::sack::ExcludeFlags::IGNORE_MODULAR_EXCLUDES;
                        } else if (param_key == "IGNORE_REGULAR_EXCLUDES") {
                            query_flags = query_flags | libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_EXCLUDES;
                        } else if (param_key == "IGNORE_REGULAR_CONFIG_EXCLUDES") {
                            query_flags = query_flags | libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_CONFIG_EXCLUDES;
                        } else if (param_key == "IGNORE_REGULAR_USER_EXCLUDES") {
                            query_flags = query_flags | libdnf5::sack::ExcludeFlags::IGNORE_REGULAR_USER_EXCLUDES;
                        } else {
                            throw JsonRequestError(fmt::format("Bad key \"{}\" for params", param_key));
                        }
                    }
                }

                auto * jfilters = get_any_object_or_null(jargs, "filters");
                if (jfilters) {
                    get_array(jargs, "filters");  // check, must be array
                }

                Action::Direction edirection = Action::Direction::ALL;
                if (jfilters) {
                    for (size_t idx = 0; idx < json_object_array_length(jfilters); ++idx) {
                        auto * jfilter = json_object_array_get_idx(jfilters, idx);
                        auto filter_key = get_string_view(jfilter, "key");
                        if (filter_key == "direction") {
                            if (domain != "trans_packages") {
                                throw JsonRequestError(
                                    "Used \"direction\" filter outside the \"trans_packages\" domain");
                            }
                            auto direction = get_string_view(jfilter, "value");
                            if (direction == "IN") {
                                edirection = Action::Direction::IN;
                            } else if (direction == "OUT") {
                                edirection = Action::Direction::OUT;
                            } else {
                                throw JsonRequestError(
                                    fmt::format("Bad \"{}\" value for \"direction\" filter", direction));
                            }
                            break;
                        }
                    }
                }

                // Initializing a new query. Query_flags or direction for transactional packages are taken into account.
                auto query = domain == "packages"
                                 ? libdnf5::rpm::PackageQuery(get_base(), query_flags)
                                 : (edirection == Action::Direction::IN
                                        ? *in_full_query
                                        : (edirection == Action::Direction::OUT ? *out_full_query : *all_full_query));

                if (jfilters) {
                    for (size_t idx = 0; idx < json_object_array_length(jfilters); ++idx) {
                        auto * jfilter = json_object_array_get_idx(jfilters, idx);
                        const auto filter_key = get_string_view(jfilter, "key");
                        auto * jvalue = get_any_object_or_null(jfilter, "value");
                        const auto value = jvalue ? std::string(get_string_view(jvalue)) : "";
                        auto * joperator = get_any_object_or_null(jfilter, "operator");
                        const auto oper = joperator ? cmp_operator_from_string(get_string_view(joperator))
                                                    : libdnf5::sack::QueryCmp::EQ;
                        if (filter_key == "direction") {
                            // The directional filter was already taken into account when initializing the query.
                        } else if (filter_key == "name") {
                            query.filter_name(value, oper);
                        } else if (filter_key == "arch") {
                            query.filter_arch(value, oper);
                        } else if (filter_key == "version") {
                            query.filter_version(value, oper);
                        } else if (filter_key == "release") {
                            query.filter_release(value, oper);
                        } else if (filter_key == "epoch") {
                            query.filter_epoch(value, oper);
                        } else if (filter_key == "nevra") {
                            query.filter_nevra(value, oper);
                        } else if (filter_key == "repo_id") {
                            query.filter_repo_id(value, oper);
                        } else if (filter_key == "available") {
                            query.filter_available();
                        } else if (filter_key == "installed") {
                            query.filter_installed();
                        } else if (filter_key == "userinstalled") {
                            query.filter_userinstalled();
                        } else if (filter_key == "installonly") {
                            query.filter_installonly();
                        } else if (filter_key == "description") {
                            query.filter_description(value, oper);
                        } else if (filter_key == "file") {
                            query.filter_file(value, oper);
                        } else if (filter_key == "upgradable") {
                            query.filter_upgradable();
                        } else if (filter_key == "upgrades") {
                            query.filter_upgrades();
                        } else if (filter_key == "downgradable") {
                            query.filter_downgradable();
                        } else if (filter_key == "downgrades") {
                            query.filter_downgrades();
                        } else {
                            throw JsonRequestError(fmt::format("Unknown package filter key \"{}\"", filter_key));
                        }
                    }
                }

                auto * jnames_val = json_object_new_array();
                json_object_object_add_ex(jreturn, domain.data(), jnames_val, JSON_C_OBJECT_ADD_CONSTANT_KEY);
                for (auto pkg : query) {
                    const auto * trans_pkg =
                        domain == "trans_packages" ? pkg_id_to_trans_pkg.at(pkg.get_id()) : nullptr;
                    auto * jobj_key_val = json_object_new_object();
                    json_object_array_add(jnames_val, jobj_key_val);
                    for (unsigned int attr_idx = 0; attr_idx < sizeof(attrs_list) / sizeof(attrs_list[0]); ++attr_idx) {
                        if (requested_attrs & (1 << attr_idx)) {
                            auto value = attrs_list[attr_idx].second(trans_pkg, pkg);
                            json_object_object_add_ex(
                                jobj_key_val,
                                attrs_list[attr_idx].first,
                                json_object_new_string(value.c_str()),
                                JSON_C_OBJECT_ADD_CONSTANT_KEY);
                        }
                    }
                }

                json_object_object_add_ex(jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                json_object_object_add_ex(
                    jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            } else if (domain == "cmdline_packages_paths") {
                if (current_hook != Hooks::PRE_ADD_CMDLINE_PACKAGES) {
                    throw JsonRequestError(
                        "Domain \"cmdline_packages_paths\" used outside the \"pre_add_cmdline_packages\" hook.");
                }
                auto * jargs = get_object(request, "args");

                auto * jpaths = json_object_new_array();
                json_object_object_add_ex(jreturn, "cmdline_packages_paths", jpaths, JSON_C_OBJECT_ADD_CONSTANT_KEY);

                auto * jfilters = get_any_object_or_null(jargs, "filters");
                if (jfilters) {
                    get_array(jargs, "filters");  // check, must be array
                    for (const auto & path : *cmdline_packages_paths) {
                        bool match_all_filters = true;
                        for (size_t idx = 0; idx < json_object_array_length(jfilters); ++idx) {
                            auto * jfilter = json_object_array_get_idx(jfilters, idx);
                            const auto filter_key = get_string_view(jfilter, "key");
                            const auto value = std::string(get_string_view(jfilter, "value"));
                            auto * joperator = get_any_object_or_null(jfilter, "operator");
                            const auto oper = joperator ? cmp_operator_from_string(get_string_view(joperator))
                                                        : libdnf5::sack::QueryCmp::EQ;
                            if (filter_key == "path") {
                                if (!sack::match_string(path, oper, value)) {
                                    match_all_filters = false;
                                    break;
                                }
                            } else {
                                throw JsonRequestError(
                                    fmt::format("Unknown cmdline package path filter key \"{}\"", filter_key));
                            }
                        }
                        if (match_all_filters) {
                            json_object_array_add(jpaths, json_object_new_string(path.c_str()));
                        }
                    }
                } else {
                    for (const auto & path : *cmdline_packages_paths) {
                        json_object_array_add(jpaths, json_object_new_string(path.c_str()));
                    }
                }
                json_object_object_add_ex(jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                json_object_object_add_ex(
                    jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            } else {
                throw JsonRequestError(fmt::format("Unknown domain \"{}\" for operation \"{}\"", domain, op));
            }
            write_json_object(jresult, out_fd);
            return;
        }
        if (op == "set") {
            auto domain = get_string_view(request, "domain");
            json_object_object_add_ex(
                jresult, "domain", json_object_new_string(domain.data()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            if (domain == "conf") {
                auto * jargs = get_object(request, "args");
                const auto key = std::string(get_string_view(jargs, "key"));
                const auto value = std::string(get_string_view(jargs, "value"));
                try {
                    const auto list_set_key_vals = set_conf(key, value);
                    auto * jkeys_val = json_object_new_array();
                    json_object_object_add_ex(jreturn, "keys_val", jkeys_val, JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    for (auto & [set_key, set_value] : list_set_key_vals) {
                        auto * jset_key_val = new_key_val_obj("key", set_key.c_str(), "value", set_value.c_str());
                        json_object_array_add(jkeys_val, jset_key_val);
                    }
                    json_object_object_add_ex(
                        jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_object_add_ex(
                        jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                } catch (const ConfigError & ex) {
                    logger->error(
                        "Actions plugin: Hook in file \"{}\" on line {}: {}",
                        command.action.file_path.string(),
                        command.action.line_number,
                        ex.what());
                    json_object_object_add_ex(
                        jresult, "status", json_object_new_string("ERROR"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_object_add_ex(
                        jresult, "message", json_object_new_string(ex.what()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                }
            } else if (domain == "vars") {
                auto * jargs = get_object(request, "args");
                auto name = std::string(get_string_view(jargs, "name"));
                auto * jvalue = get_any_object_or_null(jargs, "value");
                try {
                    auto * jnames_val = json_object_new_array();
                    json_object_object_add_ex(jreturn, "vars", jnames_val, JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    if (jvalue) {
                        const auto value = std::string(get_string_view(jvalue));
                        base.get_vars()->set(name, value, Vars::Priority::PLUGIN);
                        auto new_value = base.get_vars()->get(name).value;
                        auto * jname_val = new_key_val_obj("name", name.c_str(), "value", new_value.c_str());
                        json_object_array_add(jnames_val, jname_val);
                    } else {
                        auto vars = base.get_vars();
                        if (vars->unset(name, Vars::Priority::PLUGIN)) {
                            auto * jobj_key_val = json_object_new_object();
                            json_object_object_add_ex(
                                jobj_key_val,
                                "name",
                                json_object_new_string(name.c_str()),
                                JSON_C_OBJECT_ADD_CONSTANT_KEY);
                            json_object_array_add(jnames_val, jobj_key_val);
                        } else {
                            auto new_value = base.get_vars()->get(name).value;
                            auto * jname_val = new_key_val_obj("name", name.c_str(), "value", new_value.c_str());
                            json_object_array_add(jnames_val, jname_val);
                        }
                    }
                    json_object_object_add_ex(
                        jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_object_add_ex(
                        jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                } catch (const ReadOnlyVariableError & ex) {
                    logger->error(
                        "Actions plugin: Hook in file \"{}\" on line {}: {}",
                        command.action.file_path.string(),
                        command.action.line_number,
                        ex.what());
                    json_object_object_add_ex(
                        jresult, "status", json_object_new_string("ERROR"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_object_add_ex(
                        jresult, "message", json_object_new_string(ex.what()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                }
            } else if (domain == "actions_vars") {
                auto * jargs = get_object(request, "args");
                auto name = std::string(get_string_view(jargs, "name"));
                auto * jvalue = get_any_object_or_null(jargs, "value");
                auto * jnames_val = json_object_new_array();
                json_object_object_add_ex(jreturn, "actions_vars", jnames_val, JSON_C_OBJECT_ADD_CONSTANT_KEY);
                if (jvalue) {
                    const auto value = std::string(get_string_view(jargs, "value"));
                    tmp_variables[name] = value;
                    auto * jname_val = new_key_val_obj("name", name.c_str(), "value", value.c_str());
                    json_object_array_add(jnames_val, jname_val);
                } else {
                    tmp_variables.erase(name);
                    auto * jobj_key_val = json_object_new_object();
                    json_object_object_add_ex(
                        jobj_key_val, "name", json_object_new_string(name.c_str()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_array_add(jnames_val, jobj_key_val);
                }
                json_object_object_add_ex(jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                json_object_object_add_ex(
                    jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            } else {
                throw JsonRequestError(fmt::format("Unknown domain \"{}\" for operation \"{}\"", domain, op));
            }
            write_json_object(jresult, out_fd);
            return;
        }
        if (op == "new") {
            auto domain = get_string_view(request, "domain");
            json_object_object_add_ex(
                jresult, "domain", json_object_new_string(domain.data()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            if (domain == "repoconf") {
                if (current_hook != Hooks::REPOS_CONFIGURED) {
                    throw JsonRequestError(
                        "Injecting a new repository configuration outside the \"repos_configured\" hook");
                }
                auto * jargs = get_object(request, "args");
                auto * jkeys_val = get_array(jargs, "keys_val");

                // create repo with repo_id
                std::string repo_id;
                for (size_t idx = 0; idx < json_object_array_length(jkeys_val); ++idx) {
                    const auto jkey_val = json_object_array_get_idx(jkeys_val, idx);
                    const auto key = get_string_view(jkey_val, "key");
                    if (key == "repo_id") {
                        repo_id = get_string_view(jkey_val, "value");
                    }
                }
                if (repo_id.empty()) {
                    throw JsonRequestError("Missing \"repo_id\"");
                }

                try {
                    libdnf5::repo::RepoWeakPtr repo;
                    try {
                        auto repo_sack = base.get_repo_sack();
                        repo = repo_sack->create_repo(repo_id);
                    } catch (const libdnf5::repo::RepoError & ex) {
                        throw ConfigError(
                            fmt::format("Cannot create new repo config with id \"{}\": {}", repo_id, ex.what()));
                    }

                    // new repository is disabled by default
                    repo->get_config().get_enabled_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, false);

                    // set repository configuration
                    auto * jret_keys_val = json_object_new_array();
                    json_object_object_add_ex(jreturn, "keys_val", jret_keys_val, JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_array_add(jret_keys_val, new_key_val_obj("key", "repo_id", "value", repo_id.c_str()));
                    auto config_opts = repo->get_config().opt_binds();
                    for (size_t idx = 0; idx < json_object_array_length(jkeys_val); ++idx) {
                        const auto jkey_val = json_object_array_get_idx(jkeys_val, idx);
                        const std::string key = std::string{get_string_view(jkey_val, "key")};
                        if (key != "repo_id") {
                            const std::string value = std::string{get_string_view(jkey_val, "value")};
                            auto it = config_opts.find(key);
                            if (it == config_opts.end()) {
                                throw ConfigError(fmt::format("Unknown repo config option: {}", key));
                            }
                            try {
                                it->second.new_string(libdnf5::Option::Priority::PLUGINCONFIG, value);
                            } catch (const libdnf5::OptionError & ex) {
                                throw ConfigError(
                                    fmt::format("Cannot set repo config option \"{}={}\": {}", key, value, ex.what()));
                            }
                            auto * jset_key_val = new_key_val_obj("key", key.c_str(), "value", value.c_str());
                            json_object_array_add(jret_keys_val, jset_key_val);
                        }
                    }

                    json_object_object_add_ex(
                        jresult, "return", jreturn_owner.release(), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_object_add_ex(
                        jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                } catch (const ConfigError & ex) {
                    logger->error(
                        "Actions plugin: Hook in file \"{}\" on line {}: {}",
                        command.action.file_path.string(),
                        command.action.line_number,
                        ex.what());
                    json_object_object_add_ex(
                        jresult, "status", json_object_new_string("ERROR"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                    json_object_object_add_ex(
                        jresult, "message", json_object_new_string(ex.what()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                }
            } else {
                throw JsonRequestError(fmt::format("Unknown domain \"{}\" for operation \"{}\"", domain, op));
            }
            write_json_object(jresult, out_fd);
            return;
        }
        if (op == "log") {
            json_object_object_add_ex(jresult, "domain", json_object_new_string("log"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            auto * jargs = get_object(request, "args");
            auto level = get_string_view(jargs, "level");
            auto message = std::string(get_string_view(jargs, "message"));
            if (level == "CRITICAL") {
                logger->critical(
                    "Actions plugin: Hook in file \"{}\" on line {}: {}",
                    command.action.file_path.string(),
                    command.action.line_number,
                    message);
            } else if (level == "ERROR") {
                logger->error(
                    "Actions plugin: Hook in file \"{}\" on line {}: {}",
                    command.action.file_path.string(),
                    command.action.line_number,
                    message);
            } else if (level == "WARNING") {
                logger->warning(
                    "Actions plugin: Hook in file \"{}\" on line {}: {}",
                    command.action.file_path.string(),
                    command.action.line_number,
                    message);
            } else if (level == "NOTICE") {
                logger->notice(
                    "Actions plugin: Hook in file \"{}\" on line {}: {}",
                    command.action.file_path.string(),
                    command.action.line_number,
                    message);
            } else if (level == "INFO") {
                logger->info(
                    "Actions plugin: Hook in file \"{}\" on line {}: {}",
                    command.action.file_path.string(),
                    command.action.line_number,
                    message);
            } else if (level == "DEBUG") {
                logger->debug(
                    "Actions plugin: Hook in file \"{}\" on line {}: {}",
                    command.action.file_path.string(),
                    command.action.line_number,
                    message);
            } else if (level == "TRACE") {
                logger->trace(
                    "Actions plugin: Hook in file \"{}\" on line {}: {}",
                    command.action.file_path.string(),
                    command.action.line_number,
                    message);
            } else {
                json_object_object_add_ex(
                    jresult, "status", json_object_new_string("ERROR"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
                json_object_object_add_ex(
                    jresult,
                    "message",
                    json_object_new_string(fmt::format("Unknown log level \"{}\"", level).c_str()),
                    JSON_C_OBJECT_ADD_CONSTANT_KEY);
                write_json_object(jresult, out_fd);
                return;
            }
            json_object_object_add_ex(jresult, "status", json_object_new_string("OK"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
            write_json_object(jresult, out_fd);
            return;
        }
        throw JsonRequestError(fmt::format("Unknown operation \"{}\"", op));
    } catch (const JsonRequestError & ex) {
        logger->error(
            "Actions plugin: Hook in file \"{}\" on line {}: {}",
            command.action.file_path.string(),
            command.action.line_number,
            ex.what());
        json_object_object_add_ex(jresult, "status", json_object_new_string("ERROR"), JSON_C_OBJECT_ADD_CONSTANT_KEY);
        json_object_object_add_ex(
            jresult, "message", json_object_new_string(ex.what()), JSON_C_OBJECT_ADD_CONSTANT_KEY);
        write_json_object(jresult, out_fd);
    }
}


void Actions::execute_command(CommandToRun & command) {
    enum PipeEnd { READ = 0, WRITE = 1 };
    auto & base = get_base();

    int pipe_out_from_child[2];
    int pipe_to_child[2];
    if (pipe(pipe_to_child) == -1) {
        base.get_logger()->error("Actions plugin: Cannot create pipe: {}", std::strerror(errno));
        return;
    }
    if (pipe(pipe_out_from_child) == -1) {
        auto errnum = errno;
        close(pipe_to_child[PipeEnd::WRITE]);
        close(pipe_to_child[PipeEnd::READ]);
        base.get_logger()->error("Actions plugin: Cannot create pipe: {}", std::strerror(errnum));
        return;
    }

    auto child_pid = fork();
    if (child_pid == -1) {
        auto errnum = errno;
        close(pipe_to_child[PipeEnd::WRITE]);
        close(pipe_to_child[PipeEnd::READ]);
        close(pipe_out_from_child[PipeEnd::WRITE]);
        close(pipe_out_from_child[PipeEnd::READ]);
        base.get_logger()->error("Actions plugin: Cannot fork: {}", std::strerror(errnum));
    } else if (child_pid == 0) {
        close(pipe_to_child[PipeEnd::WRITE]);       // close writing end of the pipe on the child side
        close(pipe_out_from_child[PipeEnd::READ]);  // close reading end of the pipe on the child side

        // bind stdin of the child process to the reading end of the pipe
        if (dup2(pipe_to_child[PipeEnd::READ], fileno(stdin)) == -1) {
            base.get_logger()->error("Actions plugin: Cannot bind command stdin: {}", std::strerror(errno));
            _exit(255);
        }
        close(pipe_to_child[PipeEnd::READ]);

        // bind stdout of the child process to the writing end of the pipe
        if (dup2(pipe_out_from_child[PipeEnd::WRITE], fileno(stdout)) == -1) {
            base.get_logger()->error("Actions plugin: Cannot bind command stdout: {}", std::strerror(errno));
            _exit(255);
        }
        close(pipe_out_from_child[PipeEnd::WRITE]);

        std::vector<char *> args;
        args.reserve(command.args.size() + 1);
        for (auto & arg : command.args) {
            args.push_back(arg.data());
        }
        args.push_back(nullptr);

        execvp(command.command.c_str(), args.data());  // replace the child process with the command
        auto errnum = errno;

        std::string args_string;
        for (size_t i = 1; i < command.args.size(); ++i) {
            args_string += ' ' + command.args[i];
        }
        base.get_logger()->error(
            "Actions plugin: Cannot execute \"{}{}\": {}", command.command, args_string, std::strerror(errnum));
        _exit(255);
    } else {
        close(pipe_to_child[PipeEnd::READ]);
        close(pipe_out_from_child[PipeEnd::WRITE]);

        switch (command.action.mode) {
            case Action::Mode::PLAIN:
                close(pipe_to_child[PipeEnd::WRITE]);  // close immediately, don't send anything to child in PLAIN mode
                process_plain_communication(command, pipe_out_from_child[PipeEnd::READ]);
                break;
            case Action::Mode::JSON:
                process_json_communication(command, pipe_out_from_child[PipeEnd::READ], pipe_to_child[PipeEnd::WRITE]);
                close(pipe_to_child[PipeEnd::WRITE]);
                break;
        }

        close(pipe_out_from_child[PipeEnd::READ]);
        waitpid(child_pid, nullptr, 0);
    }
}


void Actions::on_transaction(const libdnf5::base::Transaction & transaction, const std::vector<Action> & actions) {
    if (actions.empty()) {
        return;
    }

    if (!transaction_cached) {
        trans_packages = transaction.get_transaction_packages();

        all_full_query = libdnf5::rpm::PackageQuery(get_base(), libdnf5::sack::ExcludeFlags::IGNORE_EXCLUDES, true);
        in_full_query = out_full_query = all_full_query;
        for (const auto & trans_pkg : trans_packages) {
            auto pkg = trans_pkg.get_package();
            pkg_id_to_trans_pkg[pkg.get_id()] = &trans_pkg;

            auto action = trans_pkg.get_action();
            if (transaction_item_action_is_inbound(action)) {
                in_full_query->add(pkg);
            }
            if (transaction_item_action_is_outbound(action)) {
                out_full_query->add(pkg);
            }
            all_full_query->add(pkg);
        }

        transaction_cached = true;
    }

    std::set<CommandToRun> unique_commands_to_run;  // std::set is used to detect duplicate commands

    libdnf5::ResolveSpecSettings spec_settings;
    spec_settings.set_ignore_case(false);
    spec_settings.set_with_nevra(true);
    spec_settings.set_with_provides(false);
    spec_settings.set_with_filenames(true);
    spec_settings.set_with_binaries(false);
    for (const auto & action : actions) {
        if (action.pkg_filter.empty()) {
            // action without packages - the action is called regardless of the of number of packages in the transaction
            if (auto [substituted_args, subst_error] = substitute_args(nullptr, nullptr, action); !subst_error) {
                for (auto & arg : substituted_args) {
                    unescape(arg);
                }
                CommandToRun cmd_to_run{action, action.command, std::move(substituted_args)};
                if (auto [it, inserted] = unique_commands_to_run.insert(cmd_to_run); inserted) {
                    execute_command(cmd_to_run);
                }
            }
        } else {
            // actions for packages - the action is called for each package that matches the criteria pkg_filter and direction
            auto query = action.direction == Action::Direction::IN
                             ? *in_full_query
                             : (action.direction == Action::Direction::OUT ? *out_full_query : *all_full_query);
            query.resolve_pkg_spec(action.pkg_filter, spec_settings, false);

            std::vector<CommandToRun> commands_to_run;
            for (auto pkg : query) {
                const auto * trans_pkg = pkg_id_to_trans_pkg.at(pkg.get_id());

                auto [substituted_args, subst_error] = substitute_args(trans_pkg, &pkg, action);
                if (subst_error) {
                    break;
                }

                for (auto & arg : substituted_args) {
                    unescape(arg);
                }
                CommandToRun cmd_to_run{action, action.command, substituted_args};
                if (auto [it, inserted] = unique_commands_to_run.insert(cmd_to_run); inserted) {
                    commands_to_run.push_back(std::move(cmd_to_run));
                }
            }

            // execute commands
            for (auto & cmd : commands_to_run) {
                execute_command(cmd);
            }
        }
    }
}

}  // namespace

PluginAPIVersion libdnf_plugin_get_api_version(void) {
    return PLUGIN_API_VERSION;
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
    return new Actions(data, parser);
} catch (...) {
    return nullptr;
}

void libdnf_plugin_delete_instance(plugin::IPlugin * plugin_object) {
    delete plugin_object;
}
