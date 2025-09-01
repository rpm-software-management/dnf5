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


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_ADVISORY_TMPL_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_ADVISORY_TMPL_HPP

#include "../interfaces/advisory.hpp"

namespace libdnf5::cli::output {

class IAdvisoryCollection;
template <class T>
class AdvisoryAdapter;
template <class T>
class AdvisoryCollectionAdapter;


template <class T>
class AdvisoryReferenceAdapter : public IAdvisoryReference {
public:
    AdvisoryReferenceAdapter(const T & obj) : obj{obj} {}

    AdvisoryReferenceAdapter(T && obj) : obj{std::move(obj)} {}

    std::string get_id() const override { return obj.get_id(); }

    std::string get_type() const override { return obj.get_type(); }

    const char * get_type_cstring() const override { return obj.get_type_cstring(); }

    std::string get_title() const override { return obj.get_title(); }

    std::string get_url() const override { return obj.get_url(); }

private:
    T obj;
};


template <class T>
class AdvisoryPackageAdapter : public IAdvisoryPackage {
public:
    AdvisoryPackageAdapter(const T & pkg) : pkg{pkg} {}

    AdvisoryPackageAdapter(T && pkg) : pkg{std::move(pkg)} {}

    std::string get_name() const override { return pkg.get_name(); }

    std::string get_epoch() const override { return pkg.get_epoch(); }

    std::string get_version() const override { return pkg.get_version(); }

    std::string get_release() const override { return pkg.get_release(); }

    std::string get_arch() const override { return pkg.get_arch(); }

    //std::string get_evr() const override { return pkg.get_evr(); }

    std::string get_nevra() const override { return pkg.get_nevra(); }

    std::unique_ptr<IAdvisory> get_advisory() const override {
        return std::unique_ptr<IAdvisory>(new AdvisoryAdapter(pkg.get_advisory()));
    }

private:
    T pkg;
};


template <class T>
class AdvisoryModuleAdapter : public IAdvisoryModule {
public:
    AdvisoryModuleAdapter(const T & obj) : obj{obj} {}

    AdvisoryModuleAdapter(T && obj) : obj{std::move(obj)} {}

    std::string get_name() const override { return obj.get_name(); }
    std::string get_stream() const override { return obj.get_stream(); }
    std::string get_version() const override { return obj.get_version(); }
    std::string get_context() const override { return obj.get_context(); }
    std::string get_arch() const override { return obj.get_arch(); }
    std::string get_nsvca() const override { return obj.get_nsvca(); }
    std::unique_ptr<IAdvisory> get_advisory() const override {
        return std::unique_ptr<IAdvisory>(new AdvisoryAdapter(obj.get_advisory()));
    }

private:
    T obj;
};


template <class T>
class AdvisoryCollectionAdapter : public IAdvisoryCollection {
public:
    AdvisoryCollectionAdapter(const T & obj) : obj{obj} {}

    AdvisoryCollectionAdapter(T && obj) : obj{std::move(obj)} {}

    std::vector<std::unique_ptr<IAdvisoryPackage>> get_packages() override {
        std::vector<std::unique_ptr<IAdvisoryPackage>> ret;
        auto packages = obj.get_packages();
        ret.reserve(packages.size());
        for (auto & package : packages) {
            ret.emplace_back(new AdvisoryPackageAdapter(package));
        }
        return ret;
    }

    std::vector<std::unique_ptr<IAdvisoryModule>> get_modules() override {
        std::vector<std::unique_ptr<IAdvisoryModule>> ret;
        auto modules = obj.get_modules();
        ret.reserve(modules.size());
        for (auto & module : modules) {
            ret.emplace_back(new AdvisoryModuleAdapter(module));
        }
        return ret;
    }

private:
    T obj;
};


template <class T>
class AdvisoryAdapter : public IAdvisory {
public:
    AdvisoryAdapter(const T & pkg) : pkg{pkg} {}

    AdvisoryAdapter(T && pkg) : pkg{std::move(pkg)} {}

    std::string get_name() const override { return pkg.get_name(); }

    std::string get_severity() const override { return pkg.get_severity(); }

    std::string get_type() const override { return pkg.get_type(); }

    unsigned long long get_buildtime() const override { return pkg.get_buildtime(); }

    std::string get_vendor() const override { return pkg.get_vendor(); }

    std::string get_description() const override { return pkg.get_description(); }

    std::string get_title() const override { return pkg.get_title(); }

    std::string get_status() const override { return pkg.get_status(); }

    std::string get_rights() const override { return pkg.get_rights(); }

    std::string get_message() const override { return pkg.get_message(); }

    std::vector<std::unique_ptr<IAdvisoryReference>> get_references() const override {
        std::vector<std::unique_ptr<IAdvisoryReference>> ret;
        auto references = pkg.get_references();
        ret.reserve(references.size());
        for (auto & reference : references) {
            ret.emplace_back(new AdvisoryReferenceAdapter(reference));
        }
        return ret;
    }

    std::vector<std::unique_ptr<IAdvisoryCollection>> get_collections() const override {
        std::vector<std::unique_ptr<IAdvisoryCollection>> ret;
        auto collections = pkg.get_collections();
        ret.reserve(collections.size());
        for (auto & collection : collections) {
            ret.emplace_back(new AdvisoryCollectionAdapter(collection));
        }
        return ret;
    }

private:
    T pkg;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_ADVISORY_TMPL_HPP
