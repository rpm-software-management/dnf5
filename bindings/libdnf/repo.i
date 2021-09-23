#if defined(SWIGPYTHON)
%module(package="libdnf") repo
#elif defined(SWIGPERL)
%module "libdnf::repo"
#elif defined(SWIGRUBY)
%module "libdnf/repo"
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
    } catch (const libdnf::InvalidPointer & e) {
        SWIG_exception(SWIG_NullReferenceError, e.what());
    } catch (const libdnf::RuntimeError & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

%{
    #include "libdnf/repo/config_repo.hpp"
    #include "libdnf/repo/repo.hpp"
    #include "libdnf/repo/repo_query.hpp"
    #include "libdnf/repo/repo_sack.hpp"
%}

#define CV __perl_CV

%include "libdnf/repo/config_repo.hpp"
%include "libdnf/repo/repo.hpp"

%template(RepoWeakPtr) libdnf::WeakPtr<libdnf::repo::Repo, false>;
%template(SetRepoWeakPtr) libdnf::Set<libdnf::repo::RepoWeakPtr>;
%template(SackQueryRepoWeakPtr) libdnf::sack::Query<libdnf::repo::RepoWeakPtr>;

%include "libdnf/repo/repo_query.hpp"
%template(SackRepoRepoQuery) libdnf::sack::Sack<libdnf::repo::Repo>;
%include "libdnf/repo/repo_sack.hpp"
%template(RepoSackWeakPtr) libdnf::WeakPtr<libdnf::repo::RepoSack, false>;
