#if defined(SWIGPYTHON)
%module(package="libdnf5", directors="1") base
#elif defined(SWIGPERL)
%module(directors="1") "libdnf5::base"
#elif defined(SWIGRUBY)
%module(directors="1") "libdnf5::base"
#endif

%include <exception.i>
%include <std_common.i>
%include <std_pair.i>
%include <std_vector.i>

%include "shared.i"


%import "advisory.i"
%import "common.i"
%import "comps.i"
%import "conf.i"
%import "exception.i"
%import "plugin.i"
%import "logger.i"
%import "repo.i"
%import "rpm.i"
%import "transaction.i"

%{
    #include "bindings/libdnf5/exception.hpp"

    #include "libdnf5/logger/memory_buffer_logger.hpp"
    #include "libdnf5/base/base.hpp"
    #include "libdnf5/base/text_validator_callback.hpp"
    #include "libdnf5/base/text_validator.hpp"
    #include "libdnf5/base/interaction_callbacks.hpp"
    #include "libdnf5/base/solver_problems.hpp"
    #include "libdnf5/base/log_event.hpp"
    #include "libdnf5/base/transaction.hpp"
    #include "libdnf5/base/transaction_package.hpp"
    #include "libdnf5/base/goal.hpp"
    #include "libdnf5/base/goal_elements.hpp"
%}

// Deletes any previously defined general purpose exception handler
%exception;

// Set default exception handler
%catches(libdnf5::UserAssertionError, std::runtime_error, std::out_of_range);

#define CV __perl_CV

%template(BaseWeakPtr) libdnf5::WeakPtr<libdnf5::Base, false>;
%template(VarsWeakPtr) libdnf5::WeakPtr<libdnf5::Vars, false>;

%ignore std::vector<libdnf5::plugin::PluginInfo>::insert;
%ignore std::vector<libdnf5::plugin::PluginInfo>::pop;
%ignore std::vector<libdnf5::plugin::PluginInfo>::pop_back;
%ignore std::vector<libdnf5::plugin::PluginInfo>::push;
%ignore std::vector<libdnf5::plugin::PluginInfo>::push_back;
%ignore std::vector<libdnf5::plugin::PluginInfo>::reserve;
%ignore std::vector<libdnf5::plugin::PluginInfo>::resize;
%template(VectorPluginInfo) std::vector<libdnf5::plugin::PluginInfo>;

%feature("director") libdnf5::base::InteractionCallbacks;
%feature("director") libdnf5::base::TextValidatorCallback;

// Typemaps for std::string & out_text: hides the output parameter from callers and
// appends the string to the function's return value as a second element (int, string).
// Applies to any wrapped function that has a std::string & out_text parameter.
// For Python: result, text = base.input_text(msg)
// For Perl: ($result, $text) = $base->input_text($msg)
// For Ruby: result, text = base.input_text(msg)
%typemap(in, numinputs=0) std::string & out_text (std::string temp) {
    $1 = &temp;
}

%typemap(argout) std::string & out_text {
#if defined(SWIGPYTHON)
    PyObject *o = SWIG_FromCharPtrAndSize($1->c_str(), $1->size());
    $result = SWIG_Python_AppendOutput($result, o, 0);
#elif defined(SWIGPERL)
    if (argvi >= items) {
        EXTEND(sp, 1);
    }
    ST(argvi) = SWIG_FromCharPtrAndSize($1->c_str(), $1->size());
    argvi++;
#elif defined(SWIGRUBY)
    VALUE o = SWIG_FromCharPtrAndSize($1->c_str(), $1->size());
    $result = SWIG_Ruby_AppendOutput($result, o, 0);
#endif
}

// Director typemaps for InteractionCallbacks::input_text: hide out_text from the script
// override (it's an output, not input) and translate the script's return value into the
// C++ int32_t return and out_text string.  The script override returns:
//   None/undef/nil                  -> ANSWER_DEFAULT (-3): use caller-supplied default
//   integer                         -> that value, out_text unchanged
//   (int, str) / [int, str]         -> that int value, out_text set to str
//   (int, None/undef/nil) / [int, ] -> that int value, out_text unchanged
%typemap(directorin) std::string & out_text ""

%typemap(directorout, noblock=1) int32_t libdnf5::base::InteractionCallbacks::input_text {
#if defined(SWIGPYTHON)
    if ($input == Py_None) {
        $result = libdnf5::base::ANSWER_DEFAULT;
    } else if (PyLong_Check($input)) {
        $result = static_cast<int32_t>(PyLong_AsLong($input));
    } else if (PyTuple_Check($input) && PyTuple_Size($input) == 2) {
        PyObject * answer_obj = PyTuple_GetItem($input, 0);
        PyObject * text_obj = PyTuple_GetItem($input, 1);
        if (!PyLong_Check(answer_obj)) {
            Swig::DirectorTypeMismatchException::raise("Expected (int, str) tuple: first element must be int");
        }
        $result = static_cast<int32_t>(PyLong_AsLong(answer_obj));
        if (text_obj != Py_None) {
            if (PyBytes_Check(text_obj)) {
                char * cstr;
                Py_ssize_t len;
                PyBytes_AsStringAndSize(text_obj, &cstr, &len);
                out_text.assign(cstr, len);
            } else if (PyUnicode_Check(text_obj)) {
                PyObject * bytes = PyUnicode_AsUTF8String(text_obj);
                if (bytes) {
                    char * cstr;
                    Py_ssize_t len;
                    PyBytes_AsStringAndSize(bytes, &cstr, &len);
                    out_text.assign(cstr, len);
                    Py_DECREF(bytes);
                } else {
                    Swig::DirectorTypeMismatchException::raise("Expected (int, str) tuple: string encoding error");
                }
            } else {
                Swig::DirectorTypeMismatchException::raise("Expected (int, str) tuple: second element must be str or None");
            }
        }
    } else {
        Swig::DirectorTypeMismatchException::raise("Expected None, int, or (int, str) tuple");
    }
#elif defined(SWIGPERL)
    if (!SvOK($input)) {
        $result = libdnf5::base::ANSWER_DEFAULT;
    } else if (SvIOK($input)) {
        $result = static_cast<int32_t>(SvIV($input));
    } else if (SvROK($input) && SvTYPE(SvRV($input)) == SVt_PVAV) {
        AV * av = (AV *)SvRV($input);
        if (av_len(av) != 1) {
            Swig::DirectorTypeMismatchException::raise(
                "libdnf5::base::InteractionCallbacks::input_text",
                "Expected [int, str] array ref with 2 elements");
        }
        SV ** answer_sv = av_fetch(av, 0, 0);
        SV ** text_sv = av_fetch(av, 1, 0);
        if (!answer_sv || !SvIOK(*answer_sv)) {
            Swig::DirectorTypeMismatchException::raise(
                "libdnf5::base::InteractionCallbacks::input_text",
                "Expected [int, str] array ref: first element must be int");
        }
        $result = static_cast<int32_t>(SvIV(*answer_sv));
        if (text_sv && SvOK(*text_sv)) {
            STRLEN len;
            char * cstr = SvPV(*text_sv, len);
            out_text.assign(cstr, len);
        }
    } else {
        Swig::DirectorTypeMismatchException::raise(
            "libdnf5::base::InteractionCallbacks::input_text",
            "Expected undef, int, or [int, str] array ref");
    }
#elif defined(SWIGRUBY)
    if ($input == Qnil) {
        $result = libdnf5::base::ANSWER_DEFAULT;
    } else if (rb_obj_is_kind_of($input, rb_cInteger)) {
        $result = static_cast<int32_t>(NUM2INT($input));
    } else if (rb_obj_is_kind_of($input, rb_cArray) && RARRAY_LEN($input) == 2) {
        VALUE answer_obj = rb_ary_entry($input, 0);
        VALUE text_obj = rb_ary_entry($input, 1);
        if (!rb_obj_is_kind_of(answer_obj, rb_cInteger)) {
            Swig::DirectorTypeMismatchException::raise("Expected [int, str] array: first element must be int");
        }
        $result = static_cast<int32_t>(NUM2INT(answer_obj));
        if (text_obj != Qnil) {
            const char * cstr = StringValuePtr(text_obj);
            long len = RSTRING_LEN(text_obj);
            out_text.assign(cstr, len);
        }
    } else {
        Swig::DirectorTypeMismatchException::raise("Expected nil, int, or [int, str] array");
    }
#endif
}

%include "libdnf5/base/text_validator_callback.hpp"
%include "libdnf5/base/text_validator.hpp"
%include "libdnf5/base/interaction_callbacks.hpp"

wrap_unique_ptr(InteractionCallbacksUniquePtr, libdnf5::base::InteractionCallbacks);

%include "libdnf5/base/base.hpp"

%include "libdnf5/base/solver_problems.hpp"
%include "libdnf5/base/log_event.hpp"

%include "libdnf5/base/active_transaction_info.hpp"

%ignore libdnf5::base::TransactionError;
%include "libdnf5/base/transaction.hpp"

%template(VectorLogEvent) std::vector<libdnf5::base::LogEvent>;

%include "libdnf5/base/transaction_environment.hpp"
%include "libdnf5/base/transaction_group.hpp"
%include "libdnf5/base/transaction_package.hpp"

%template(VectorBaseTransactionEnvironment) std::vector<libdnf5::base::TransactionEnvironment>;
%template(VectorBaseTransactionGroup) std::vector<libdnf5::base::TransactionGroup>;
%template(VectorBaseTransactionPackage) std::vector<libdnf5::base::TransactionPackage>;

%include "libdnf5/base/goal.hpp"
%include "libdnf5/base/goal_elements.hpp"

// Add attributes for getters/setters in Python.
// See 'common.i' for more info.
#if defined(SWIGPYTHON)
%pythoncode %{
common.create_attributes_from_getters_and_setters(ResolveSpecSettings)
common.create_attributes_from_getters_and_setters(GoalJobSettings)
%}
#endif

// Deletes any previously defined catches
%catches();
