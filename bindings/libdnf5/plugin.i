#if defined(SWIGPYTHON)
%module(package="libdnf5", directors="1") plugin
#elif defined(SWIGPERL)
%module "libdnf5::plugin"
#elif defined(SWIGRUBY)
%module "libdnf5::plugin"
#endif

%include <exception.i>
%include <stdint.i>
%include <std_common.i>

%include <shared.i>

%import "conf.i"

%{
    #include "libdnf5/plugin/iplugin.hpp"
    #include "libdnf5/plugin/plugin_info.hpp"
%}

#define CV __perl_CV

%include "libdnf5/plugin/plugin_version.hpp"

%extend libdnf5::plugin::Version {
    Version(std::uint16_t major, std::uint16_t minor, std::uint16_t micro) {
        libdnf5::plugin::Version * ver = new libdnf5::plugin::Version({major, minor, micro});
        return ver;
    }
}

%ignore PluginError;
%ignore libdnf_plugin_get_api_version;
%ignore libdnf_plugin_get_name;
%ignore libdnf_plugin_get_version;
%ignore libdnf_plugin_new_instance;
%ignore libdnf_plugin_delete_instance;
%feature("director") IPlugin;
%include "libdnf5/plugin/iplugin.hpp"

%include "libdnf5/plugin/plugin_info.hpp"
