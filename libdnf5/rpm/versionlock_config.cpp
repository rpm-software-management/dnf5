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

#include "libdnf5/rpm/versionlock_config.hpp"

#include "utils/string.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/utils/fs/file.hpp"

#include <fmt/format.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <toml.hpp>

#include <map>
#include <utility>

namespace {
// supported config file version
const std::string_view CONFIG_FILE_VERSION = "1.0";
}  // namespace


namespace toml {

template <>
struct from<libdnf5::rpm::VersionlockCondition> {
    static libdnf5::rpm::VersionlockCondition from_toml(const toml::value & val) {
        auto key = toml::find_or<std::string>(val, "key", "");
        auto comparator = toml::find_or<std::string>(val, "comparator", "");
        auto value = toml::find_or<std::string>(val, "value", "");

        libdnf5::rpm::VersionlockCondition condition(key, comparator, value);

        return condition;
    }
};

template <>
struct into<libdnf5::rpm::VersionlockCondition> {
#ifdef TOML11_COMPAT
    static toml::value into_toml(const libdnf5::rpm::VersionlockCondition & condition) {
        toml::value res;
#else
    template <typename TC>
    static toml::basic_value<TC> into_toml(const libdnf5::rpm::VersionlockCondition & condition) {
        toml::basic_value<TC> res;
#endif  // #ifdef TOML11_COMPAT

        res["key"] = condition.get_key_str();
        res["comparator"] = condition.get_comparator_str();
        res["value"] = condition.get_value();

        return res;
    }
};

template <>
struct from<libdnf5::rpm::VersionlockPackage> {
    static libdnf5::rpm::VersionlockPackage from_toml(const toml::value & val) {
        auto name = toml::find_or<std::string>(val, "name", "");
        libdnf5::rpm::VersionlockPackage package(
            name, toml::find_or<std::vector<libdnf5::rpm::VersionlockCondition>>(val, "conditions", {}));
        auto comment = toml::find_or<std::string>(val, "comment", "");
        if (!comment.empty()) {
            package.set_comment(comment);
        }

        return package;
    }
};

template <>
struct into<libdnf5::rpm::VersionlockPackage> {
#ifdef TOML11_COMPAT
    static toml::value into_toml(const libdnf5::rpm::VersionlockPackage & package) {
        toml::value res;
#else
    template <typename TC>
    static toml::basic_value<TC> into_toml(const libdnf5::rpm::VersionlockPackage & package) {
        toml::basic_value<TC> res;
#endif  // #ifdef TOML11_COMPAT

        res["name"] = package.get_name();
        auto comment = package.get_comment();
        if (!comment.empty()) {
            res["comment"] = package.get_comment();
        }
        res["conditions"] = package.get_conditions();

        return res;
    }
};

}  // namespace toml


namespace libdnf5::rpm {

const std::map<std::string, libdnf5::sack::QueryCmp> VersionlockCondition::VALID_COMPARATORS = {
    {"=", libdnf5::sack::QueryCmp::EQ},
    {"==", libdnf5::sack::QueryCmp::EQ},
    {"<", libdnf5::sack::QueryCmp::LT},
    {"<=", libdnf5::sack::QueryCmp::LTE},
    {">", libdnf5::sack::QueryCmp::GT},
    {">=", libdnf5::sack::QueryCmp::GTE},
    {"<>", libdnf5::sack::QueryCmp::NEQ},
    {"!=", libdnf5::sack::QueryCmp::NEQ},
};

VersionlockCondition::VersionlockCondition(
    const std::string & key_str, const std::string & comparator_str, const std::string & value)
    : valid(true),
      key_str(key_str),
      comparator_str(comparator_str),
      value(value) {
    // check that condition key is present and valid
    if (key_str == "epoch") {
        key = Keys::EPOCH;
    } else if (key_str == "evr") {
        key = Keys::EVR;
    } else if (key_str == "arch") {
        key = Keys::ARCH;
    } else {
        valid = false;
        if (key_str.empty()) {
            errors.emplace_back("missing condition key");
        } else {
            errors.emplace_back(fmt::format("invalid condition key \"{}\"", key_str));
        }
    }

    // check that condition comparison operator is present and valid
    if (VALID_COMPARATORS.contains(comparator_str)) {
        comparator = VALID_COMPARATORS.at(comparator_str);
    } else {
        valid = false;
        if (comparator_str.empty()) {
            errors.emplace_back("missing condition comparison operator");
        } else {
            errors.emplace_back(fmt::format("invalid condition comparison operator \"{}\"", comparator_str));
        }
    }

    // check that condition value is present
    if (value.empty()) {
        valid = false;
        errors.emplace_back("missing condition value");
    }

    if (valid) {
        // additional checks for specific keys
        switch (key) {
            case Keys::EPOCH:
                // the epoch condition requires a valid integer as a value
                try {
                    std::stoul(value);
                } catch (...) {
                    valid = false;
                    errors.emplace_back("epoch condition value needs to be an unsigned integer");
                }
                break;
            case Keys::ARCH:
                if (comparator != libdnf5::sack::QueryCmp::EQ && comparator != libdnf5::sack::QueryCmp::NEQ) {
                    valid = false;
                    errors.emplace_back("\"arch\" condition only supports \"=\" and \"!=\" comparison operators");
                }
            default:
                break;
        }
    }
}

std::string VersionlockCondition::to_string(bool with_errors) const {
    std::string str = fmt::format("{} {} {}", key_str, comparator_str, value);
    if (!valid && with_errors) {
        str += fmt::format(" # {}", utils::string::join(errors, ", "));
    }
    return str;
}


VersionlockPackage::VersionlockPackage(
    std::string_view name, std::vector<libdnf5::rpm::VersionlockCondition> && conditions)
    : valid(true),
      name(name),
      conditions(std::move(conditions)) {
    // check that package name is present
    if (name.empty()) {
        valid = false;
        errors.emplace_back("missing package name");
    }
    // package without any condition doesn't lock anything
    if (this->conditions.empty()) {
        valid = false;
        errors.emplace_back("missing package conditions");
    }
}

void VersionlockPackage::set_comment(std::string_view comment) {
    this->comment = comment;
}

void VersionlockPackage::add_condition(VersionlockCondition && condition) {
    conditions.emplace_back(std::move(condition));
}

std::string VersionlockPackage::to_string(bool with_errors, bool with_comment) const {
    std::string str;
    if (with_comment && !comment.empty()) {
        str += fmt::format("# {}\n", comment);
    }
    str += fmt::format("Package name: {}", name);
    if (!valid and with_errors) {
        str += fmt::format(" # entry is invalid: {}", utils::string::join(errors, ", "));
    }
    for (const auto & cond : conditions) {
        str += "\n";
        str += cond.to_string(with_errors);
    }
    return str;
}


VersionlockConfig::VersionlockConfig(const std::filesystem::path & path) : path(path) {
    if (!std::filesystem::exists(path)) {
        return;
    }

    auto toml_value = toml::parse(this->path);

    if (!toml_value.contains("version")) {
        // TODO(mblaha) Log unversioned versionlock file?
        return;
    } else if (toml::find<std::string>(toml_value, "version") != CONFIG_FILE_VERSION) {
        // TODO(mblaha) Log unsupported versionlock file version?
        return;
    }

    packages = toml::find_or<std::vector<VersionlockPackage>>(toml_value, "packages", {});
}

#ifdef TOML11_COMPAT
template <typename T>
static toml::value make_top_value(const std::string & key, const T & value) {
    return toml::value({{key, value}, { "version", CONFIG_FILE_VERSION }});
}

static std::string toml_format(const toml::value & value) {
    return toml::format<toml::discard_comments, std::map, std::vector>(value);
}
#else
template <typename T>
static toml::ordered_value make_top_value(const std::string & key, const T & value) {
    return toml::ordered_value({{key, value}, {"version", CONFIG_FILE_VERSION}});
}

static std::string toml_format(const toml::ordered_value & value) {
    return toml::format(value);
}
#endif  // #ifdef TOML11_COMPAT

void VersionlockConfig::save() {
    std::error_code ecode;
    std::filesystem::create_directories(path.parent_path(), ecode);
    if (ecode) {
        throw FileSystemError(errno, path, M_("{}"), ecode.message());
    }

    utils::fs::File(path, "w").write(toml_format(make_top_value("packages", packages)));
}

}  // namespace libdnf5::rpm
