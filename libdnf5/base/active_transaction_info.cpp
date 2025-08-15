// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/base/active_transaction_info.hpp"

#include <toml.hpp>

#include <ctime>

namespace libdnf5::base {

class ActiveTransactionInfo::Impl {
public:
    pid_t pid = -1;
    std::string description;
    std::string comment;
    time_t start_time = 0;
};

ActiveTransactionInfo::ActiveTransactionInfo() : p_impl(new Impl()) {}

ActiveTransactionInfo::ActiveTransactionInfo(const ActiveTransactionInfo & src) : p_impl(new Impl(*src.p_impl)) {}
ActiveTransactionInfo & ActiveTransactionInfo::operator=(const ActiveTransactionInfo & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}

ActiveTransactionInfo::ActiveTransactionInfo(ActiveTransactionInfo && src) noexcept = default;
ActiveTransactionInfo & ActiveTransactionInfo::operator=(ActiveTransactionInfo && src) noexcept = default;

ActiveTransactionInfo::~ActiveTransactionInfo() = default;

const std::string & ActiveTransactionInfo::get_description() const noexcept {
    return p_impl->description;
}

const std::string & ActiveTransactionInfo::get_comment() const noexcept {
    return p_impl->comment;
}

pid_t ActiveTransactionInfo::get_pid() const noexcept {
    return p_impl->pid;
}

time_t ActiveTransactionInfo::get_start_time() const noexcept {
    return p_impl->start_time;
}

void ActiveTransactionInfo::set_description(const std::string & description) {
    p_impl->description = description;
}

void ActiveTransactionInfo::set_comment(const std::string & comment) {
    p_impl->comment = comment;
}

void ActiveTransactionInfo::set_pid(pid_t pid) {
    p_impl->pid = pid;
}

void ActiveTransactionInfo::set_start_time(time_t start_time) {
    p_impl->start_time = start_time;
}

std::string ActiveTransactionInfo::to_toml() const {
    toml::value res;
    res["description"] = get_description();
    res["comment"] = get_comment();
    res["pid"] = get_pid();
    res["start_time"] = get_start_time();
    return toml::format(res);
}

ActiveTransactionInfo ActiveTransactionInfo::from_toml(const std::string & toml_string) {
    ActiveTransactionInfo info;

    try {
        auto data = toml::parse_str(toml_string);
        info.set_description(toml::find_or<std::string>(data, "description", ""));
        info.set_comment(toml::find_or<std::string>(data, "comment", ""));
        info.set_pid(toml::find_or<pid_t>(data, "pid", -1));
        info.set_start_time(toml::find_or<time_t>(data, "start_time", 0));
    } catch (const toml::syntax_error &) {
        // Return default/empty ActiveTransactionInfo on TOML syntax errors
    } catch (const toml::type_error &) {
        // Return default/empty ActiveTransactionInfo on TOML type errors
    }

    return info;
}

}  // namespace libdnf5::base
