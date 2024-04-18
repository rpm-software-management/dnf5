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
#include <libdnf5/base/base.hpp>
#include <libdnf5/base/transaction.hpp>
#include <libdnf5/common/exception.hpp>
#include <libdnf5/plugin/iplugin.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
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
constexpr plugin::Version PLUGIN_VERSION{0, 3, 0};

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
};


// Represents one command to run
struct CommandToRun {
    [[nodiscard]] bool operator<(const CommandToRun & other) const noexcept;
    std::string command;
    std::vector<std::string> args;
};


class Actions : public plugin::IPlugin {
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

    void pre_base_setup() override { on_base_setup(pre_base_setup_actions); }

    void post_base_setup() override { on_base_setup(post_base_setup_actions); }

    void pre_transaction(const libdnf5::base::Transaction & transaction) override {
        on_transaction(transaction, pre_trans_actions);
    }

    void post_transaction(const libdnf5::base::Transaction & transaction) override {
        on_transaction(transaction, post_trans_actions);
    }

private:
    void parse_action_files();
    void on_base_setup(const std::vector<Action> & trans_actions);
    void on_transaction(const libdnf5::base::Transaction & transaction, const std::vector<Action> & trans_actions);
    void execute_command(CommandToRun & command);

    [[nodiscard]] std::pair<std::string, bool> substitute(
        const libdnf5::base::TransactionPackage * trans_pkg,
        const libdnf5::rpm::Package * pkg,
        std::string_view in,
        std::filesystem::path file,
        int line_number);

    [[nodiscard]] std::pair<std::vector<std::string>, bool> substitute_args(
        const libdnf5::base::TransactionPackage * trans_pkg, const libdnf5::rpm::Package * pkg, const Action & action);

    void process_command_output_line(std::string_view line);

    // Parsed actions for individual hooks
    std::vector<Action> pre_base_setup_actions;
    std::vector<Action> post_base_setup_actions;
    std::vector<Action> pre_trans_actions;
    std::vector<Action> post_trans_actions;

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
            auto config_opts = base.get_config().opt_binds();
            auto it = config_opts.find(std::string(var_name.substr(5)));
            if (it != config_opts.end()) {
                var_value = it->second.get_value_string();
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

void Actions::on_base_setup(const std::vector<Action> & trans_actions) {
    if (trans_actions.empty()) {
        return;
    }

    std::set<CommandToRun> unique_commands_to_run;  // std::set is used to detect duplicate commands

    for (const auto & action : trans_actions) {
        if (auto [substituted_args, subst_error] = substitute_args(nullptr, nullptr, action); !subst_error) {
            for (auto & arg : substituted_args) {
                unescape(arg);
            }
            CommandToRun cmd_to_run{action.command, std::move(substituted_args)};
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
                } else {
                    throw ActionsPluginError(
                        M_("Error in file \"{}\" on line {}: Unknown option \"{}\""), path.native(), line_number, opt);
                }
            }
            if (!action_enabled) {
                continue;
            }

            enum class Hooks { PRE_BASE_SETUP, POST_BASE_SETUP, PRE_TRANS, POST_TRANS } hook;
            if (line.starts_with("pre_base_setup:")) {
                hook = Hooks::PRE_BASE_SETUP;
            } else if (line.starts_with("post_base_setup:")) {
                hook = Hooks::POST_BASE_SETUP;
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
                case Hooks::PRE_TRANS:
                    pre_trans_actions.emplace_back(std::move(act));
                    break;
                case Hooks::POST_TRANS:
                    post_trans_actions.emplace_back(std::move(act));
            }
        }
    }
}

void Actions::process_command_output_line(std::string_view line) {
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
            "Actions plugin: Syntax error: Missing equal sign (=) in command output line \"{}\"", std::string(line));
        return;
    }
    if (line.starts_with("conf.")) {
        std::string var_name(line.substr(5, eq_pos - 5));
        std::string var_value(line.substr(eq_pos + 1));
        auto config_opts = base.get_config().opt_binds();
        auto it = config_opts.find(var_name);
        if (it == config_opts.end()) {
            base.get_logger()->error("Actions plugin: Command returns unknown config option \"{}\"", var_name);
            return;
        }
        try {
            it->second.new_string(libdnf5::Option::Priority::PLUGINCONFIG, var_value);
        } catch (libdnf5::OptionError & ex) {
            base.get_logger()->error(
                "Actions plugin: Cannot set config value returned by command \"{}={}\": {}",
                var_name,
                var_value,
                std::string(ex.what()));
        }
    } else if (line.starts_with("var.")) {
        std::string var_name(line.substr(4, eq_pos - 4));
        std::string var_value(line.substr(eq_pos + 1));
        base.get_vars()->set(var_name, var_value, libdnf5::Vars::Priority::PLUGIN);
    } else {
        base.get_logger()->error(
            "Actions plugin: Command output line not in correct format (has to start with \"tmp.\" or \"conf.\" or "
            "\"var.\"): \"{}\"",
            std::string(line));
    }
}

void Actions::execute_command(CommandToRun & command) {
    auto & base = get_base();

    int pipe_out_from_child[2];
    int pipe_to_child[2];
    if (pipe(pipe_to_child) == -1) {
        base.get_logger()->error("Actions plugin: Cannot create pipe: {}", std::strerror(errno));
        return;
    }
    if (pipe(pipe_out_from_child) == -1) {
        auto errnum = errno;
        close(pipe_to_child[1]);
        close(pipe_to_child[0]);
        base.get_logger()->error("Actions plugin: Cannot create pipe: {}", std::strerror(errnum));
        return;
    }

    auto child_pid = fork();
    if (child_pid == -1) {
        auto errnum = errno;
        close(pipe_to_child[1]);
        close(pipe_to_child[0]);
        close(pipe_out_from_child[1]);
        close(pipe_out_from_child[0]);
        base.get_logger()->error("Actions plugin: Cannot fork: {}", std::strerror(errnum));
    } else if (child_pid == 0) {
        close(pipe_to_child[1]);        // close writing end of the pipe on the child side
        close(pipe_out_from_child[0]);  // close reading end of the pipe on the child side

        // bind stdin of the child process to the reading end of the pipe
        if (dup2(pipe_to_child[0], fileno(stdin)) == -1) {
            base.get_logger()->error("Actions plugin: Cannot bind command stdin: {}", std::strerror(errno));
            _exit(255);
        }
        close(pipe_to_child[0]);

        // bind stdout of the child process to the writing end of the pipe
        if (dup2(pipe_out_from_child[1], fileno(stdout)) == -1) {
            base.get_logger()->error("Actions plugin: Cannot bind command stdout: {}", std::strerror(errno));
            _exit(255);
        }
        close(pipe_out_from_child[1]);

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
        close(pipe_to_child[0]);
        close(pipe_to_child[1]);

        close(pipe_out_from_child[1]);
        char read_buf[256];
        std::string input;
        std::size_t num_tested_chars = 0;
        do {
            auto len = read(pipe_out_from_child[0], read_buf, sizeof(read_buf));
            if (len > 0) {
                std::size_t line_begin_pos = 0;
                input.append(read_buf, static_cast<std::size_t>(len));
                std::string_view input_view(input);
                do {
                    auto line_end_pos = input_view.find('\n', num_tested_chars);
                    if (line_end_pos == std::string::npos) {
                        num_tested_chars = input_view.size();
                    } else {
                        process_command_output_line(input_view.substr(line_begin_pos, line_end_pos - line_begin_pos));
                        num_tested_chars = line_begin_pos = line_end_pos + 1;
                    }
                } while (num_tested_chars < input_view.size());

                // shift - erase processed lines from the input buffer
                input.erase(0, line_begin_pos);
                num_tested_chars -= line_begin_pos;
                line_begin_pos = 0;
            } else {
                if (!input.empty()) {
                    process_command_output_line(input);
                }
                break;
            }
        } while (true);
        close(pipe_out_from_child[0]);

        waitpid(child_pid, nullptr, 0);
    }
}

void Actions::on_transaction(
    const libdnf5::base::Transaction & transaction, const std::vector<Action> & trans_actions) {
    if (trans_actions.empty()) {
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
    for (const auto & action : trans_actions) {
        if (action.pkg_filter.empty()) {
            // action without packages - the action is called regardless of the of number of packages in the transaction
            if (auto [substituted_args, subst_error] = substitute_args(nullptr, nullptr, action); !subst_error) {
                for (auto & arg : substituted_args) {
                    unescape(arg);
                }
                CommandToRun cmd_to_run{action.command, std::move(substituted_args)};
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
                CommandToRun cmd_to_run{action.command, substituted_args};
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
