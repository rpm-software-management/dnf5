%module(package="libdnf") conf

#if defined(SWIGPYTHON)
%import "common.i"
#elif defined(SWIGRUBY)
%import(module="libdnf/common") "common.i"
#elif defined(SWIGPERL)
%include "std_vector.i"
%import(module="libdnf::common") "common.i"
#endif

%{
    #include "libdnf/conf/option_child.hpp"
    #include "libdnf/conf/config_main.hpp"
    #include "libdnf/conf/config_parser.hpp"
%}

#define CV __perl_CV
#define final

%ignore libdnf::Option::Exception;
%ignore libdnf::Option::InvalidValue;
%include "libdnf/conf/option.hpp"

%ignore libdnf::OptionBool::InvalidValue;
%include "libdnf/conf/option_bool.hpp"

%ignore libdnf::OptionString::InvalidValue;
%ignore libdnf::OptionString::NotAllowedValue;
%ignore libdnf::OptionString::ValueNotSet;

%ignore libdnf::OptionEnum::InvalidValue;
%ignore libdnf::OptionEnum::NotAllowedValue;
%include "libdnf/conf/option_enum.hpp"
%template(OptionEnumString) libdnf::OptionEnum<std::string>;

%ignore libdnf::OptionNumber::InvalidValue;
%ignore libdnf::OptionNumber::NotAllowedValue;
%include "libdnf/conf/option_number.hpp"
%template(OptionNumberInt32) libdnf::OptionNumber<std::int32_t>;
%template(OptionNumberUInt32) libdnf::OptionNumber<std::uint32_t>;
%template(OptionNumberInt64) libdnf::OptionNumber<std::int64_t>;
%template(OptionNumberUInt64) libdnf::OptionNumber<std::uint64_t>;
%template(OptionNumberFloat) libdnf::OptionNumber<float>;

%include "libdnf/conf/option_seconds.hpp"
%include "libdnf/conf/option_string.hpp"
%include "libdnf/conf/option_string_list.hpp"
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
%ignore libdnf::OptionBinds::Exception;
%ignore libdnf::OptionBinds::OptionNotFound;
%ignore libdnf::OptionBinds::OptionAlreadyExists;
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

%include "libdnf/conf/config_parser.hpp"

%include "libdnf/conf/vars.hpp"
