#if defined(SWIGPYTHON)
%module(package="libdnf") conf
#elif defined(SWIGPERL)
%module "libdnf::conf"
#elif defined(SWIGRUBY)
%module "libdnf/conf"
#endif

%include <exception.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"

%exception {
    try {
        $action
    } catch (const libdnf::InvalidPointerError & e) {
        SWIG_exception(SWIG_NullReferenceError, e.what());
    } catch (const std::runtime_error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

%{
    #include "libdnf/conf/option_child.hpp"
    #include "libdnf/conf/config_main.hpp"
    #include "libdnf/conf/config_parser.hpp"
    #include "libdnf/common/weak_ptr.hpp"
%}

#define CV __perl_CV
#define final

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

%ignore libdnf::OptionPathNotFoundError;
%include "libdnf/conf/option_path.hpp"

%include "libdnf/conf/option_child.hpp"
%template(OptionChildBool) libdnf::OptionChild<libdnf::OptionBool>;
%template(OptionChildString) libdnf::OptionChild<libdnf::OptionString>;
%template(OptionChildStringList) libdnf::OptionChild<libdnf::OptionStringList>;
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

%include "libdnf/conf/config.hpp"
%include "libdnf/conf/config_main.hpp"

%ignore libdnf::ConfigParserError;
%ignore ConfigParserSectionNotFoundError;
%ignore ConfigParserOptionNotFoundError;
%include "libdnf/conf/config_parser.hpp"

%include "libdnf/conf/vars.hpp"
