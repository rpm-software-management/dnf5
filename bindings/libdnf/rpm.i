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

%{
    #include "libdnf/rpm/checksum.hpp"
    #include "libdnf/conf/config_main.hpp"
    #include "libdnf/rpm/config_repo.hpp"
    #include "libdnf/rpm/package.hpp"
    #include "libdnf/rpm/package_set.hpp"
    #include "libdnf/rpm/package_set_iterator.hpp"
    #include "libdnf/rpm/reldep.hpp"
    #include "libdnf/rpm/reldep_list.hpp"
    #include "libdnf/rpm/reldep_list_iterator.hpp"
    #include "libdnf/rpm/repo.hpp"
    #include "libdnf/rpm/repo_query.hpp"
    #include "libdnf/rpm/repo_sack.hpp"
    #include "libdnf/rpm/solv_query.hpp"
    #include "libdnf/rpm/solv_sack.hpp"
%}

#define CV __perl_CV

%include "libdnf/rpm/checksum.hpp"
%include "libdnf/rpm/solv_sack.hpp"
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

%ignore libdnf::rpm::SolvQuery::SolvQuery(SolvQuery && src);
%include "libdnf/rpm/solv_query.hpp"

%include "libdnf/rpm/config_repo.hpp"
%include "libdnf/rpm/repo.hpp"

%template(RepoWeakPtr) libdnf::WeakPtr<libdnf::rpm::Repo, false>;
%template(SetRepoWeakPtr) libdnf::Set<libdnf::rpm::RepoWeakPtr>;
%template(SackQueryRepoWeakPtr) libdnf::sack::Query<libdnf::rpm::RepoWeakPtr>;

%include "libdnf/rpm/repo_query.hpp"
%template(SackRepoRepoQuery) libdnf::sack::Sack<libdnf::rpm::Repo, libdnf::rpm::RepoQuery>;
%include "libdnf/rpm/repo_sack.hpp"

add_iterator(PackageSet)
add_iterator(ReldepList)
