#if defined(SWIGPYTHON)
%module(package="libdnf", directors="1") repo
#elif defined(SWIGPERL)
%module "libdnf::repo"
#elif defined(SWIGRUBY)
%module(directors="1") "libdnf/repo"
#endif

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"
%import "conf.i"

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
    #include "libdnf/repo/config_repo.hpp"
    #include "libdnf/repo/package_downloader.hpp"
    #include "libdnf/repo/repo.hpp"
    #include "libdnf/repo/repo_callbacks.hpp"
    #include "libdnf/repo/repo_query.hpp"
    #include "libdnf/repo/repo_sack.hpp"
%}

#define CV __perl_CV

%feature("valuewrapper") Package;

%include "libdnf/repo/config_repo.hpp"

%ignore PackageDownloadError;
%feature("director") PackageDownloadCallbacks;
%include "libdnf/repo/package_downloader.hpp"
wrap_unique_ptr(PackageDownloadCallbacksUniquePtr, libdnf::repo::PackageDownloadCallbacks);

%include "libdnf/repo/repo.hpp"

%include "libdnf/repo/repo_weak.hpp"
%template(RepoWeakPtr) libdnf::WeakPtr<libdnf::repo::Repo, false>;
%template(SetConstIteratorRepoWeakPtr) libdnf::SetConstIterator<libdnf::repo::RepoWeakPtr>;
%template(SetRepoWeakPtr) libdnf::Set<libdnf::repo::RepoWeakPtr>;
%template(SackQueryRepoWeakPtr) libdnf::sack::Query<libdnf::repo::RepoWeakPtr>;

%feature("director") RepoCallbacks;
%include "libdnf/repo/repo_callbacks.hpp"
wrap_unique_ptr(RepoCallbacksUniquePtr, libdnf::repo::RepoCallbacks);

%include "libdnf/repo/repo_query.hpp"
%template(SackRepoRepoQuery) libdnf::sack::Sack<libdnf::repo::Repo>;
%include "libdnf/repo/repo_sack.hpp"
%template(RepoSackWeakPtr) libdnf::WeakPtr<libdnf::repo::RepoSack, false>;

add_iterator(SetRepoWeakPtr)
