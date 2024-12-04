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
    } catch (const libdnf5::UserAssertionError & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const libdnf5::Error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const std::out_of_range & e) {
        SWIG_exception(SWIG_IndexError, e.what());
    } catch (const std::runtime_error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

%{
    #include "libdnf5/conf/const.hpp"
    #include "libdnf5/conf/option_child.hpp"
    #include "libdnf5/conf/config_main.hpp"
    #include "libdnf5/conf/config_parser.hpp"
    #include "libdnf5/common/weak_ptr.hpp"
    #include "libdnf5/logger/log_router.hpp"
    #include "libdnf5/logger/memory_buffer_logger.hpp"
    #include "libdnf5/version.hpp"
%}

#define CV __perl_CV
#define final

wrap_unique_ptr(StringUniquePtr, std::string);

%include "libdnf5/version.hpp"

%include "libdnf5/conf/const.hpp"

%ignore libdnf5::OptionError;
%ignore libdnf5::OptionInvalidValueError;
%ignore libdnf5::OptionValueNotAllowedError;
%ignore libdnf5::OptionValueNotSetError;
%include "libdnf5/conf/option.hpp"
%include "libdnf5/conf/option_bool.hpp"
%include "libdnf5/conf/option_enum.hpp"

%include "libdnf5/conf/option_number.hpp"
%template(OptionNumberInt32) libdnf5::OptionNumber<std::int32_t>;
%template(OptionNumberUInt32) libdnf5::OptionNumber<std::uint32_t>;
%template(OptionNumberInt64) libdnf5::OptionNumber<std::int64_t>;
%template(OptionNumberUInt64) libdnf5::OptionNumber<std::uint64_t>;
%template(OptionNumberFloat) libdnf5::OptionNumber<float>;

%include "libdnf5/conf/option_seconds.hpp"
%include "libdnf5/conf/option_string.hpp"
%include "libdnf5/conf/option_string_list.hpp"
%template(OptionStringSet) libdnf5::OptionStringContainer<std::set<std::string>, false>;
%template(OptionStringList) libdnf5::OptionStringContainer<std::vector<std::string>, false>;
%template(OptionStringAppendSet) libdnf5::OptionStringContainer<std::set<std::string>, true>;
%template(OptionStringAppendList) libdnf5::OptionStringContainer<std::vector<std::string>, true>;

%ignore libdnf5::OptionPathNotFoundError;
%include "libdnf5/conf/option_path.hpp"

%include "libdnf5/conf/option_child.hpp"
%template(OptionChildBool) libdnf5::OptionChild<libdnf5::OptionBool>;
%template(OptionChildString) libdnf5::OptionChild<libdnf5::OptionString>;
%template(OptionChildStringList) libdnf5::OptionChild<libdnf5::OptionStringList>;
%template(OptionChildStringSet) libdnf5::OptionChild<libdnf5::OptionStringSet>;
%template(OptionChildStringAppendList) libdnf5::OptionChild<libdnf5::OptionStringAppendList>;
%template(OptionChildStringAppendSet) libdnf5::OptionChild<libdnf5::OptionStringAppendSet>;
%template(OptionChildNumberInt32) libdnf5::OptionChild<libdnf5::OptionNumber<std::int32_t>>;
%template(OptionChildNumberUInt32) libdnf5::OptionChild<libdnf5::OptionNumber<std::uint32_t>>;
%template(OptionChildNumberFloat) libdnf5::OptionChild<libdnf5::OptionNumber<float>>;
%template(OptionChildEnum) libdnf5::OptionChild<libdnf5::OptionEnum>;
%template(OptionChildSeconds) libdnf5::OptionChild<libdnf5::OptionSeconds>;


%rename (OptionBinds_Item) libdnf5::OptionBinds::Item;
%ignore libdnf5::OptionBindsError;
%ignore libdnf5::OptionBindsOptionNotFoundError;
%ignore libdnf5::OptionBindsOptionAlreadyExistsError;
%ignore libdnf5::OptionBinds::add(const std::string & id, Option & option,
    Item::NewStringFunc new_string_func, Item::GetValueStringFunc get_value_string_func, bool add_value);
%ignore libdnf5::OptionBinds::begin;
%ignore libdnf5::OptionBinds::cbegin;
%ignore libdnf5::OptionBinds::end;
%ignore libdnf5::OptionBinds::cend;
%ignore libdnf5::OptionBinds::find;
%include "libdnf5/conf/option_binds.hpp"

%ignore libdnf5::ConfigParserError;
%ignore libdnf5::InaccessibleConfigError;
%ignore libdnf5::MissingConfigError;
%ignore libdnf5::InvalidConfigError;
%ignore ConfigParserSectionNotFoundError;
%ignore ConfigParserOptionNotFoundError;
%include "libdnf5/conf/config_parser.hpp"

%newobject libdnf5::Vars::detect_release;
%extend libdnf5::Vars {
  static char * detect_release(const BaseWeakPtr & base, const std::string & install_root_path) {
    std::unique_ptr<std::string> release = libdnf5::Vars::detect_release(base, install_root_path);
    if (release) {
      char * release_cstr = new char[release->size() + 1];
      strcpy(release_cstr, release->c_str());
      return release_cstr;
    }
    return nullptr;
  }
};
%ignore libdnf5::Vars::detect_release;
%ignore libdnf5::ReadOnlyVariableError;
%include "libdnf5/conf/vars.hpp"
%template(VarsWeakPtr) libdnf5::WeakPtr<libdnf5::Vars, false>;

%include "libdnf5/conf/config.hpp"
%include "libdnf5/conf/config_main.hpp"

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
    try:
        return getattr(config_object, option_name)().get_value()
    except RuntimeError:
        return None

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

%exception;  // beware this resets all exception handlers if you import this file after defining any
