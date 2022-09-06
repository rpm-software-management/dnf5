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

#ifndef LIBDNF_MODULE_MODULE_ITEM_HPP
#define LIBDNF_MODULE_MODULE_ITEM_HPP

#include "libdnf/module/module_dependency.hpp"
#include "libdnf/module/module_item_container_weak.hpp"
#include "libdnf/module/module_profile.hpp"

#include <string>
#include <vector>

struct _ModulemdModuleStream;

class ModuleTest;


namespace libdnf::module {


struct ModuleItemId {
public:
    ModuleItemId() = default;
    explicit ModuleItemId(int id) : id(id) {}

    bool operator==(const ModuleItemId & other) const noexcept { return id == other.id; }
    bool operator!=(const ModuleItemId & other) const noexcept { return id != other.id; }

    // Corresponds to solvable id
    int id{0};
};


// Represents one modulemd document (uniquely described by name:stream:version:context:arch, but there can theoretically be more objects with the same NSVCA)
// @replaces libdnf:module/ModuleItem.hpp:class:ModuleItem
class ModuleItem {
public:
    bool operator==(const ModuleItem & r) const;

    ~ModuleItem();

    /// @return The module name.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getNameCStr()
    std::string get_name() const;

    /// @return The stream name.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getStream()
    std::string get_stream() const;

    /// @return The version.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getVersionNum()
    long long get_version() const;
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getVersion()
    std::string get_version_str() const;

    /// @return The context.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getContext()
    std::string get_context() const;

    /// @return The arch.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getArch()
    std::string get_arch() const;

    /// @return The "name:stream:version:context:arch" string.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getFullIdentifier()
    std::string get_full_identifier() const;

    /// @return The summary of this ModuleItem.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getSummary()
    std::string get_summary() const;

    /// @return The description of this ModuleItem.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getDescription()
    std::string get_description() const;

    // TODO(pkratoch): Translations for summary and description

    /// @return The list of RPM NEVRAs in this module.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getArtifacts()
    std::vector<std::string> get_artifacts() const;

    /// @return Sorted list of RPM names that are demodularized.
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getDemodularizedRpms()
    std::vector<std::string> get_demodularized_rpms() const;

    /// @return The list of ModuleProfiles matched by name (possibly a globby pattern).
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getProfiles(const std::string &name)
    std::vector<ModuleProfile> get_profiles(const std::string & name) const;
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getProfiles()
    std::vector<ModuleProfile> get_profiles() const;

    // TODO(pkratoch): Similar implementation as in ModuleItemContainer::getDefaultProfiles().
    /// @return The default profiles.
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getDefaultProfile()
    std::vector<ModuleProfile> get_default_profiles() const;

    /// Get the `static_context` value (needed for proper behaviour of modular solver).
    ///
    /// @return The value of the `static_context` flag in modulemd. If `true`, the context string
    ///         is static and represents a build and runtime configuration for this stream.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getStaticContext()
    bool get_static_context() const;

    /// @return Module dependencies.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getRequires(bool removePlatform=false)
    std::vector<ModuleDependency> get_module_dependencies(bool remove_platform = false) const;

    /// @return The repo id.
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getRepoID()
    const std::string & get_repo_id() const { return repo_id; };

    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getId()
    ModuleItemId get_id() const { return id; };

    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getYaml()
    std::string get_yaml() const;

private:
    friend class ModuleMetadata;
    friend ::ModuleTest;

    ModuleItem(_ModulemdModuleStream * md_stream, const ModuleItemContainerWeakPtr & module_item_container);
    ModuleItem(const ModuleItem & mpkg);
    ModuleItem & operator=(const ModuleItem & mpkg);
    ModuleItem(ModuleItem && mpkg) = default;
    ModuleItem & operator=(ModuleItem && mpkg) = default;

    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getNameCStr()
    const char * get_name_cstr() const;
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getStreamCStr()
    const char * get_stream_cstr() const;
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getContextCStr()
    const char * get_context_cstr() const;
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getArchCStr()
    const char * get_arch_cstr() const;
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getNameStream()
    std::string get_name_stream() const;
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getNameStreamVersion()
    std::string get_name_stream_version() const;

    std::vector<ModuleProfile> get_profiles_internal(const char * name) const;

    static std::vector<ModuleDependency> get_module_dependencies(
        _ModulemdModuleStream * md_stream, bool remove_platform);

    // TODO(pkratoch): Make this private once it's not used in tests.
    /// @return The string representing module dependencies (e.g. "ninja;platform:[f29]" or "nodejs:[-11]").
    /// @param remove_platform When true, the method will not return dependencies with stream "platform"
    ///                        (default is false).
    /// @since 5.0
    //
    // @replaces libdnf:module/ModuleItem.hpp:method:ModuleItem.getRequires(bool removePlatform=false)
    static std::string get_module_dependencies_string(_ModulemdModuleStream * md_stream, bool remove_platform);
    std::string get_module_dependencies_string(bool remove_platform = false) const;

    static std::string get_name_stream(_ModulemdModuleStream * md_stream);

    // Corresponds to one yaml document
    _ModulemdModuleStream * md_stream;

    ModuleItemContainerWeakPtr module_item_container;
    ModuleItemId id;
    std::string repo_id;
};


// TODO(pkratoch): Compare olso their modular sacks.
inline bool ModuleItem::operator==(const ModuleItem & r) const {
    return id == r.id;
}


inline std::vector<ModuleProfile> ModuleItem::get_profiles(const std::string & name) const {
    return get_profiles_internal(name.c_str());
}


inline std::vector<ModuleProfile> ModuleItem::get_profiles() const {
    return get_profiles_internal(nullptr);
}


inline std::vector<ModuleDependency> ModuleItem::get_module_dependencies(bool remove_platform) const {
    return get_module_dependencies(md_stream, remove_platform);
}


inline std::string ModuleItem::get_module_dependencies_string(bool remove_platform) const {
    return get_module_dependencies_string(md_stream, remove_platform);
}


inline std::string ModuleItem::get_name_stream() const {
    return get_name_stream(md_stream);
}


}  // namespace libdnf::module


#endif  // LIBDNF_MODULE_MODULE_ITEM_HPP
