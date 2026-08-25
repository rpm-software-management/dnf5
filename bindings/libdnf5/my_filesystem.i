%include <std_string.i>
%include <std_vector.i>

%{
#include <filesystem>
#include <string>
#include <vector>
%}

// Integration of std::filesystem::path with SWIG internal container traits
// Wrap traits in a SWIG fragment that depends on SWIG's internal "StdTraits"
%fragment(SWIG_Traits_frag(std::filesystem::path), "header", fragment="StdTraits", fragment="SWIG_From_std_string", fragment="SWIG_AsPtr_std_string") {
namespace swig {

template <>
struct traits<std::filesystem::path> {
    typedef value_category category;
    static const char * type_name() { return "std::filesystem::path"; }
};


template <>
struct traits_from<std::filesystem::path> {
    typedef value_category category;
    static SWIG_Object from(const std::filesystem::path & val) {
        return swig::from(val.string());
    }
};


template <>
struct traits_asptr<std::filesystem::path> {
    typedef pointer_category category;
    static int asptr(SWIG_Object obj, std::filesystem::path ** val) {
        std::string * buf = nullptr;
        int res = swig::asptr(obj, &buf);
        if (SWIG_IsOK(res) && buf) {
            if (val) {
                *val = new std::filesystem::path(*buf);
            }
            if (SWIG_IsNewObj(res)) {
                delete buf;
            }
            return SWIG_NEWOBJ;
        }
        if (SWIG_IsNewObj(res) && buf) {
            delete buf;
        }
        return res;
    }
};

}  // namespace swig
}  // fragment


// --- Input typemaps ---

%typemap(in, fragment="SWIG_AsPtr_std_string") std::filesystem::path (std::string *buf = nullptr, int res = 0) {
    res = SWIG_AsPtr_std_string($input, &buf);
    if (!SWIG_IsOK(res) || !buf) {
        %argument_fail(res, "$1_type", $symname, $argnum);
    }
    $1 = *buf;
    if (SWIG_IsNewObj(res)) delete buf;
}

%typemap(in, fragment="SWIG_AsPtr_std_string") const std::filesystem::path & (std::string *buf = nullptr, int res = 0, std::filesystem::path temp) {
    res = SWIG_AsPtr_std_string($input, &buf);
    if (!SWIG_IsOK(res) || !buf) {
        %argument_fail(res, "$1_type", $symname, $argnum);
    }
    temp = *buf;
    $1 = &temp;
    if (SWIG_IsNewObj(res)) delete buf;
}

// Input typemap for const std::filesystem::path * (allows NULL / None / nil / undef)
%typemap(in, fragment="SWIG_AsPtr_std_string") const std::filesystem::path * (std::string *buf = nullptr, int res = 0, std::filesystem::path temp) {
    res = $input ? SWIG_AsPtr_std_string($input, &buf) : SWIG_ERROR;
    if (SWIG_IsOK(res) && buf) {
        temp = *buf;
        $1 = &temp;
        if (SWIG_IsNewObj(res)) delete buf;
    } else {
        // Check if input represents a null pointer / None / nil / undef
        void *p = nullptr;
        if (SWIG_IsOK(SWIG_ConvertPtr($input, &p, 0, 0)) && !p) {
            $1 = nullptr;
        } else {
            %argument_fail(res, "$1_type", $symname, $argnum);
        }
    }
}


// --- Typecheck for function overloading resolution (const inputs only) ---

%typemap(typecheck, precedence=SWIG_TYPECHECK_STRING, fragment="SWIG_AsPtr_std_string")  std::filesystem::path, const std::filesystem::path & {
    int res = SWIG_AsPtr_std_string($input, (std::string**)(nullptr));
    $1 = SWIG_CheckState(res) ? 1 : 0;
}

// Typecheck for const std::filesystem::path * (accepts strings or null objects)
%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER, fragment="SWIG_AsPtr_std_string") const std::filesystem::path * {
    int res = SWIG_AsPtr_std_string($input, (std::string**)(nullptr));
    if (SWIG_CheckState(res)) {
        $1 = 1;
    } else {
        void *p = nullptr;
        $1 = (SWIG_IsOK(SWIG_ConvertPtr($input, &p, 0, 0)) && !p) ? 1 : 0;
    }
}


// --- Output typemaps (supports values, const/non-const references, and pointers) ---
%typemap(out, fragment="SWIG_From_std_string") std::filesystem::path {
    %set_output(SWIG_From_std_string($1.string()));
}

%typemap(out, fragment="SWIG_From_std_string") const std::filesystem::path & {
    %set_output(SWIG_From_std_string($1->string()));
}

%typemap(out, fragment="SWIG_From_std_string") const std::filesystem::path * {
    if ($1) {
        %set_output(SWIG_From_std_string($1->string()));
    }
}
