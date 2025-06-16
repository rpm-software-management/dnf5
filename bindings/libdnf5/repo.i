#if defined(SWIGPYTHON)
%module("threads"=1, package="libdnf5", directors="1") repo
%nothread;
#elif defined(SWIGPERL)
%module "libdnf5::repo"
#elif defined(SWIGRUBY)
%module(directors="1") "libdnf5::repo"
#endif

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%include "shared.i"

%import "common.i"
%import "conf.i"
%import "exception.i"

#if SWIG_VERSION == 0x040200
// https://github.com/swig/swig/issues/2744
%fragment("SwigPyIterator_T");
#endif

%{
    #include "bindings/libdnf5/exception.hpp"

    #include "libdnf5/logger/log_router.hpp"
    #include "libdnf5/logger/memory_buffer_logger.hpp"
    #include "libdnf5/logger/rotating_file_logger.hpp"
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

// Deletes any previously defined exception handlers
%exception;
%catches();

%{
static inline void * integer_to_void_ptr(int value) noexcept {
    return reinterpret_cast<void *>(static_cast<intptr_t>(value));
}

static inline int void_ptr_to_int(void * value) noexcept {
    return static_cast<int>(reinterpret_cast<intptr_t>(value));
}
%}

// Set default exception handler
%catches(libdnf5::UserAssertionError, std::runtime_error, std::out_of_range);

%feature("valuewrapper") Package;

%include "libdnf5/repo/config_repo.hpp"

%typemap(directorin, noblock=1) void * {
    $input = SWIG_From_int(void_ptr_to_int($1));
}

%typemap(directorout, noblock=1) void * {
    int swig_val;
    int swig_res = SWIG_AsVal_int($1, &swig_val);
    if (SWIG_IsOK(swig_res)) {
        $result = integer_to_void_ptr(swig_val);
#if defined(SWIGPYTHON)
    } else if (Py_IsNone($1)) {
        $result = 0;
#endif
    } else {
        Swig::DirectorTypeMismatchException::raise(SWIG_ErrorType(SWIG_ArgError(swig_res)), "in output value of type '""int""'");
    }
}

%typemap(in, noblock=1) void * {
    {
        int swig_val;
        int swig_res = SWIG_AsVal_int($input, &swig_val);
        if (!SWIG_IsOK(swig_res)) {
            Swig::DirectorTypeMismatchException::raise(SWIG_ErrorType(SWIG_ArgError(swig_res)), "in input value of type '""int""'");
        }
        $1 = integer_to_void_ptr(swig_val);
    }
}

%typemap(out, noblock=1) void * {
    $result = SWIG_From_int(void_ptr_to_int($1));
}

%feature("director") DownloadCallbacks;
%include "libdnf5/repo/download_callbacks.hpp"
%typemap(directorin) void *;
%typemap(directorout) void *;
%typemap(in) void *;
%typemap(out) void *;
wrap_unique_ptr(DownloadCallbacksUniquePtr, libdnf5::repo::DownloadCallbacks);

%extend libdnf5::repo::FileDownloader {
    void add(libdnf5::repo::RepoWeakPtr & repo, const std::string & url, const std::string & destination, int user_data = 0) {
        $self->add(repo, url, destination, integer_to_void_ptr(user_data));
    }
    void add(const std::string & url, const std::string & destination, int user_data = 0) {
        $self->add(url, destination, integer_to_void_ptr(user_data));
    }
};
%ignore libdnf5::repo::FileDownloader::add;
%ignore FileDownloadError;
%include "libdnf5/repo/file_downloader.hpp"

%extend libdnf5::repo::PackageDownloader {
    void add(const libdnf5::rpm::Package & package, int user_data = 0) {
        $self->add(package, integer_to_void_ptr(user_data));
    }
    void add(const libdnf5::rpm::Package & package, const std::string & destination, int user_data = 0) {
        $self->add(package, destination, integer_to_void_ptr(user_data));
    }
};
%ignore libdnf5::repo::PackageDownloader::add;
%ignore PackageDownloadError;
%include "libdnf5/repo/package_downloader.hpp"

%ignore RepoCacheError;
%include "libdnf5/repo/repo_cache.hpp"

%extend libdnf5::repo::Repo {
  void set_user_data(int user_data) noexcept {
      $self->set_user_data(integer_to_void_ptr(user_data));
  }
  int get_user_data() const noexcept {
    return void_ptr_to_int($self->get_user_data());
  }
};
%ignore libdnf5::repo::Repo::set_user_data;
%ignore libdnf5::repo::Repo::get_user_data;
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

#if defined(SWIGPYTHON)
%thread;
#endif
%include "libdnf5/repo/repo_sack.hpp"
%template(RepoSackWeakPtr) libdnf5::WeakPtr<libdnf5::repo::RepoSack, false>;

add_iterator(SetRepoWeakPtr)

// Add configuration options attributes for Python.
// See 'conf.i' for more info.
#if defined(SWIGPYTHON)
%pythoncode %{
conf.create_config_option_attributes(ConfigRepo)
%}
// Add configuration options iterator for Python.
add_config_iterator(ConfigRepo)
#endif

// Add attributes for getters/setters in Python.
// See 'common.i' for more info.
#if defined(SWIGPYTHON)
%pythoncode %{
common.create_attributes_from_getters_and_setters(RepoCacheRemoveStatistics)
%}
#endif

// Deletes any previously defined catches
%catches();
