#if defined(SWIGPYTHON)
%module(package="libdnf5") conf
#elif defined(SWIGPERL)
%module "libdnf5::conf"
#elif defined(SWIGRUBY)
%module "libdnf5/conf"
#endif

%include <exception.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"
%import "logger.i"

%exception {
    try {
        $action
    } catch (const std::out_of_range & e) {
        SWIG_exception(SWIG_IndexError, e.what());
    } catch (const std::runtime_error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

%{
    #include "libdnf/conf/const.hpp"
    #include "libdnf/conf/option_child.hpp"
    #include "libdnf/conf/config_main.hpp"
    #include "libdnf/conf/config_parser.hpp"
    #include "libdnf/common/weak_ptr.hpp"
    #include "libdnf/logger/log_router.hpp"
    #include "libdnf/logger/memory_buffer_logger.hpp"
    #include "libdnf/version.hpp"
%}

#define CV __perl_CV
#define final

%include "libdnf/version.hpp"

%include "libdnf/conf/const.hpp"

%ignore libdnf::OptionError;
%ignore libdnf::OptionInvalidValueError;
%ignore libdnf::OptionValueNotAllowedError;
%ignore libdnf::OptionValueNotSetError;
%include "libdnf/conf/option.hpp"
%include "libdnf/conf/option_bool.hpp"
%include "libdnf/conf/option_enum.hpp"
%template(OptionEnumString) libdnf::OptionEnum<std::string>;

%include "libdnf/conf/option_number.hpp"
%template(OptionNumberInt32) libdnf::OptionNumber<std::int32_t>;
%template(OptionNumberUInt32) libdnf::OptionNumber<std::uint32_t>;
%template(OptionNumberInt64) libdnf::OptionNumber<std::int64_t>;
%template(OptionNumberUInt64) libdnf::OptionNumber<std::uint64_t>;
%template(OptionNumberFloat) libdnf::OptionNumber<float>;

%include "libdnf/conf/option_seconds.hpp"
%include "libdnf/conf/option_string.hpp"
%include "libdnf/conf/option_string_list.hpp"
%template(OptionStringSet) libdnf::OptionStringContainer<std::set<std::string>>;
%template(OptionStringList) libdnf::OptionStringContainer<std::vector<std::string>>;

%ignore libdnf::OptionPathNotFoundError;
%include "libdnf/conf/option_path.hpp"

%include "libdnf/conf/option_child.hpp"
%template(OptionChildBool) libdnf::OptionChild<libdnf::OptionBool>;
%template(OptionChildString) libdnf::OptionChild<libdnf::OptionString>;
%template(OptionChildStringList) libdnf::OptionChild<libdnf::OptionStringList>;
%template(OptionChildStringSet) libdnf::OptionChild<libdnf::OptionStringSet>;
%template(OptionChildNumberInt32) libdnf::OptionChild<libdnf::OptionNumber<std::int32_t>>;
%template(OptionChildNumberUInt32) libdnf::OptionChild<libdnf::OptionNumber<std::uint32_t>>;
%template(OptionChildNumberFloat) libdnf::OptionChild<libdnf::OptionNumber<float>>;
%template(OptionChildEnumString) libdnf::OptionChild<libdnf::OptionEnum<std::string>>;
%template(OptionChildSeconds) libdnf::OptionChild<libdnf::OptionSeconds>;


%rename (OptionBinds_Item) libdnf::OptionBinds::Item;
%ignore libdnf::OptionBindsError;
%ignore libdnf::OptionBindsOptionNotFoundError;
%ignore libdnf::OptionBindsOptionAlreadyExistsError;
%ignore libdnf::OptionBinds::add(const std::string & id, Option & option,
    Item::NewStringFunc new_string_func, Item::GetValueStringFunc get_value_string_func, bool add_value);
%ignore libdnf::OptionBinds::begin;
%ignore libdnf::OptionBinds::cbegin;
%ignore libdnf::OptionBinds::end;
%ignore libdnf::OptionBinds::cend;
%ignore libdnf::OptionBinds::find;
%include "libdnf/conf/option_binds.hpp"

%ignore libdnf::ConfigParserError;
%ignore libdnf::InaccessibleConfigError;
%ignore libdnf::MissingConfigError;
%ignore libdnf::InvalidConfigError;
%ignore ConfigParserSectionNotFoundError;
%ignore ConfigParserOptionNotFoundError;
%include "libdnf/conf/config_parser.hpp"

%include "libdnf/conf/vars.hpp"

%include "libdnf/conf/config.hpp"
%include "libdnf/conf/config_main.hpp"

// The following adds shortcuts in Python for getting or setting
// the configuration options using the configuration class attributes.
//
// To access all methods from the option, use standard getters as in C++ API.
//
// Example with the 'installroot':
//
// base = libdnf5.base.Base()
// config = base.get_config()
// config.installroot = '/tmp/installroot'
// config.get_installroot_option().test('relative/path')
//
#if defined(SWIGPYTHON)
%pythoncode %{
import re

def _config_option_getter(config_object, option_name):
    return getattr(config_object, option_name)().get_value()

def _config_option_setter(config_object, option_name, value):
    getattr(config_object, option_name)().set(value)

def create_config_option_attributes(cls):
    for attr in dir(cls):
        option_getter_match = re.search(r'get_(\w+)_option', attr)
        if option_getter_match:
            option_name = option_getter_match.group(1)
            setattr(cls, option_name, property(
                lambda self, attr=attr: _config_option_getter(self, attr),
                lambda self, value, attr=attr: _config_option_setter(self, attr, value)
            ))

create_config_option_attributes(ConfigMain)
%}
#endif
