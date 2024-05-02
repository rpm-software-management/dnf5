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
%include "libdnf5/repo/download_callbacks.hpp"
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
