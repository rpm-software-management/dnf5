#if defined(SWIGPYTHON)
%module(package="libdnf5", directors="1") repo
#elif defined(SWIGPERL)
%module "libdnf5::repo"
#elif defined(SWIGRUBY)
%module(directors="1") "libdnf5/repo"
#endif

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"
%import "conf.i"

#if SWIG_VERSION == 0x040200
// https://github.com/swig/swig/issues/2744
%fragment("SwigPyIterator_T");
#endif

%exception {
    try {
        $action
    } catch (const libdnf5::UserAssertionError & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const libdnf5::Error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const std::runtime_error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

%{
    #include "libdnf5/logger/log_router.hpp"
    #include "libdnf5/logger/memory_buffer_logger.hpp"
    #include "libdnf5/repo/config_repo.hpp"
    #include "libdnf5/repo/download_callbacks.hpp"
    #include "libdnf5/repo/file_downloader.hpp"
    #include "libdnf5/repo/package_downloader.hpp"
    #include "libdnf5/repo/repo.hpp"
    #include "libdnf5/repo/repo_cache.hpp"
    #include "libdnf5/repo/repo_callbacks.hpp"
    #include "libdnf5/repo/repo_query.hpp"
    #include "libdnf5/repo/repo_sack.hpp"
%}

#define CV __perl_CV

%feature("valuewrapper") Package;

%include "libdnf5/repo/config_repo.hpp"

%feature("director") DownloadCallbacks;

%typemap(directorin, noblock=1) void * user_cb_data {
    $input = SWIG_From_int(static_cast<int>(reinterpret_cast<intptr_t>($1)));
}

%typemap(directorout, noblock=1) void * {
    int swig_val;
    int swig_res = SWIG_AsVal_int($1, &swig_val);
    if (SWIG_IsOK(swig_res)) {
        $result = reinterpret_cast<void *>(swig_val);
#if defined(SWIGPYTHON)
    } else if (Py_IsNone($1)) {
        $result = 0;
#endif
    } else {
        Swig::DirectorTypeMismatchException::raise(SWIG_ErrorType(SWIG_ArgError(swig_res)), "in output value of type '""int""'");
    }
}

%typemap(in, noblock=1) void * user_cb_data {
    {
        int swig_val;
        int swig_res = SWIG_AsVal_int($input, &swig_val);
        if (!SWIG_IsOK(swig_res)) {
            Swig::DirectorTypeMismatchException::raise(SWIG_ErrorType(SWIG_ArgError(swig_res)), "in input value of type '""int""'");
        }
        $1 = reinterpret_cast<void *>(swig_val);
    }
}

%typemap(out, noblock=1) void * {
    $result = SWIG_From_int(static_cast<int>(reinterpret_cast<intptr_t>($1)));
}

%include "libdnf5/repo/download_callbacks.hpp"
%typemap(directorin) void *;
%typemap(directorout) void * user_cb_data;
%typemap(in) void * user_cb_data;
%typemap(out) void *;
wrap_unique_ptr(DownloadCallbacksUniquePtr, libdnf5::repo::DownloadCallbacks);

%ignore FileDownloadError;
%include "libdnf5/repo/file_downloader.hpp"

%ignore PackageDownloadError;
%include "libdnf5/repo/package_downloader.hpp"

%ignore RepoCacheError;
%include "libdnf5/repo/repo_cache.hpp"

%include "libdnf5/repo/repo.hpp"

%include "libdnf5/repo/repo_weak.hpp"
%template(RepoWeakPtr) libdnf5::WeakPtr<libdnf5::repo::Repo, false>;
%template(SetConstIteratorRepoWeakPtr) libdnf5::SetConstIterator<libdnf5::repo::RepoWeakPtr>;
%template(SetRepoWeakPtr) libdnf5::Set<libdnf5::repo::RepoWeakPtr>;
%template(SackQueryRepoWeakPtr) libdnf5::sack::Query<libdnf5::repo::RepoWeakPtr>;

%feature("director") RepoCallbacks;
%include "libdnf5/repo/repo_callbacks.hpp"
wrap_unique_ptr(RepoCallbacksUniquePtr, libdnf5::repo::RepoCallbacks);

%include "libdnf5/repo/repo_query.hpp"
%template(SackRepoRepoQuery) libdnf5::sack::Sack<libdnf5::repo::Repo>;
%include "libdnf5/repo/repo_sack.hpp"
%template(RepoSackWeakPtr) libdnf5::WeakPtr<libdnf5::repo::RepoSack, false>;

add_iterator(SetRepoWeakPtr)

// Add configuration options attributes for Python.
// See 'conf.i' for more info.
#if defined(SWIGPYTHON)
%pythoncode %{
conf.create_config_option_attributes(ConfigRepo)
%}
#endif

// Add attributes for getters/setters in Python.
// See 'common.i' for more info.
#if defined(SWIGPYTHON)
%pythoncode %{
common.create_attributes_from_getters_and_setters(RepoCacheRemoveStatistics)
%}
#endif
