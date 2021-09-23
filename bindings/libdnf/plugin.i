#if defined(SWIGPYTHON)
%module(package="libdnf", directors="1") plugin
#elif defined(SWIGPERL)
%module "libdnf::plugin"
#elif defined(SWIGRUBY)
%module "libdnf/plugin"
#endif

%include <exception.i>
%include <stdint.i>
%include <std_common.i>

%include <shared.i>

%{
    #include "libdnf/plugin/iplugin.hpp"
    #include "libdnf/plugin/plugins.hpp"
%}

#define CV __perl_CV

%ignore libdnf_plugin_get_api_version;
%ignore libdnf_plugin_get_name;
%ignore libdnf_plugin_get_version;
%ignore libdnf_plugin_new_instance;
%ignore libdnf_plugin_delete_instance;
%feature("director") IPlugin;
%include "libdnf/plugin/iplugin.hpp"

%extend libdnf::plugin::Version {
    Version(std::uint16_t major, std::uint16_t minor, std::uint16_t micro) {
        libdnf::plugin::Version * ver = new libdnf::plugin::Version({major, minor, micro});
        return ver;
    }
}

%include "libdnf/plugin/plugins.hpp"
