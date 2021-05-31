#if defined(SWIGPYTHON)
%module(package="libdnf") rpm
#elif defined(SWIGPERL)
%module "libdnf::rpm"
#elif defined(SWIGRUBY)
%module "libdnf/rpm"
#endif

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

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

    #include "libdnf/advisory/advisory_module.hpp"
    #include "libdnf/advisory/advisory_query.hpp"
    #include "libdnf/rpm/checksum.hpp"
    #include "libdnf/conf/config_main.hpp"
    #include "libdnf/rpm/config_repo.hpp"
    #include "libdnf/rpm/nevra.hpp"
    #include "libdnf/rpm/package.hpp"
    #include "libdnf/rpm/package_query.hpp"
    #include "libdnf/rpm/package_sack.hpp"
    #include "libdnf/rpm/package_set.hpp"
    #include "libdnf/rpm/package_set_iterator.hpp"
    #include "libdnf/rpm/reldep.hpp"
    #include "libdnf/rpm/reldep_list.hpp"
    #include "libdnf/rpm/reldep_list_iterator.hpp"
    #include "libdnf/rpm/repo.hpp"
    #include "libdnf/rpm/repo_query.hpp"
    #include "libdnf/rpm/repo_sack.hpp"
%}

#define CV __perl_CV

%include "libdnf/rpm/checksum.hpp"
%include "libdnf/rpm/nevra.hpp"

%template(VectorNevra) std::vector<libdnf::rpm::Nevra>;

%include "libdnf/rpm/package_sack.hpp"
%template(PackageSackWeakPtr) libdnf::WeakPtr<libdnf::rpm::PackageSack, false>;


%ignore libdnf::advisory::AdvisorySack::new_query();
%include "libdnf/advisory/advisory_sack.hpp"
%template(AdvisorySackWeakPtr) libdnf::WeakPtr<libdnf::advisory::AdvisorySack, false>;


%include "libdnf/rpm/reldep.hpp"

%rename(next) libdnf::rpm::ReldepListIterator::operator++();
%rename(value) libdnf::rpm::ReldepListIterator::operator*();
%include "libdnf/rpm/reldep_list_iterator.hpp"
%include "libdnf/rpm/reldep_list.hpp"
%include "libdnf/rpm/package.hpp"

%rename(next) libdnf::rpm::PackageSetIterator::operator++();
%rename(value) libdnf::rpm::PackageSetIterator::operator*();
%include "libdnf/rpm/package_set_iterator.hpp"
%include "libdnf/rpm/package_set.hpp"

%ignore libdnf::rpm::PackageQuery::PackageQuery(PackageQuery && src);
%include "libdnf/rpm/package_query.hpp"

%include "libdnf/rpm/config_repo.hpp"
%include "libdnf/rpm/repo.hpp"

%template(RepoWeakPtr) libdnf::WeakPtr<libdnf::repo::Repo, false>;
%template(SetRepoWeakPtr) libdnf::Set<libdnf::repo::RepoWeakPtr>;
%template(SackQueryRepoWeakPtr) libdnf::sack::Query<libdnf::repo::RepoWeakPtr>;

%include "libdnf/rpm/repo_query.hpp"
%template(SackRepoRepoQuery) libdnf::sack::Sack<libdnf::repo::Repo, libdnf::repo::RepoQuery>;
%include "libdnf/rpm/repo_sack.hpp"
%template(RepoSackWeakPtr) libdnf::WeakPtr<libdnf::repo::RepoSack, false>;

add_iterator(PackageSet)
add_iterator(ReldepList)
