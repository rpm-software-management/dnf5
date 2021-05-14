#if defined(SWIGPYTHON)
%module(package="libdnf") common
#elif defined(SWIGPERL)
%module "libdnf::common"
#elif defined(SWIGRUBY)
%module "libdnf/common"
#endif

#define SWIGWORDSIZE64

%include <exception.i>
%include <stdint.i>
%include <std_map.i>
%include <std_pair.i>
#if defined(SWIGPYTHON) || defined(SWIGRUBY)
%include <std_set.i>
#endif
%include <std_string.i>
%include <std_vector.i>

%{
    #include "libdnf/common/weak_ptr.hpp"
%}
%include "libdnf/common/weak_ptr.hpp"

// Cant use %include <catch_error.i> here, SWIG includes each file only once,
// but the exception handler actually doesnt get registered when this file is
// %imported (as opposed to %included).
%exception {
    try {
        $action
    } catch (const libdnf::InvalidPointer & e) {
        SWIG_exception(SWIG_NullReferenceError, e.what());
    } catch (const std::out_of_range & e) {
        SWIG_exception(SWIG_IndexError, e.what());
    } catch (const std::exception & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

%template(VectorString) std::vector<std::string>;
#if defined(SWIGPYTHON) || defined(SWIGRUBY)
%template(SetString) std::set<std::string>;
#endif
%template(PairStringString) std::pair<std::string, std::string>;
%template(VectorPairStringString) std::vector<std::pair<std::string, std::string>>;
%template(MapStringString) std::map<std::string, std::string>;
%template(MapStringMapStringString) std::map<std::string, std::map<std::string, std::string>>;
%template(MapStringPairStringString) std::map<std::string, std::pair<std::string, std::string>>;

namespace std {
  %feature("novaluewrapper") unique_ptr;
  template <typename Type>
  struct unique_ptr {

     explicit unique_ptr(Type * ptr);
     unique_ptr(unique_ptr && right);
     template<class Type2, Class Del2> unique_ptr(unique_ptr<Type2, Del2> && right);
     unique_ptr(const unique_ptr & right) = delete;

     Type * operator->() const;
     Type * release();
     void reset(Type * __p = nullptr);
     void swap(unique_ptr & __u);
     Type * get() const;
     operator bool() const;

     ~unique_ptr();
  };
}

%define wrap_unique_ptr(Name, Type)
  %newobject std::unique_ptr<Type>::release;
  %apply SWIGTYPE *DISOWN {Type * ptr};
  %template(Name) std::unique_ptr<Type>;
  %clear Type * ptr;

  %typemap(out) std::unique_ptr<Type> %{
    $result = SWIG_NewPointerObj(new $1_ltype(std::move($1)), $&1_descriptor, SWIG_POINTER_OWN);
  %}

%enddef

#if defined(SWIGPYTHON)
%pythoncode %{
class Iterator:
    def __init__(self, container, begin, end):
        # Store a reference to the iterated container to prevent the Python
        # gargabe collector from freeing the container from memory.
        self.container = container

        self.cur = begin
        self.end = end

    def __iter__(self):
        return self

    def __next__(self):
        if self.cur == self.end:
            raise StopIteration
        else:
            value = self.cur.value()
            self.cur.next()
            return value
%}
#endif

%define add_iterator(ClassName)
#if defined(SWIGPYTHON)
%pythoncode %{
def ClassName##__iter__(self):
    return common.Iterator(self, self.begin(), self.end())
ClassName.__iter__ = ClassName##__iter__
del ClassName##__iter__
%}
#endif
%enddef


%{
    #include "libdnf/common/sack/query.hpp"
    #include "libdnf/common/sack/query_cmp.hpp"
    #include "libdnf/common/sack/sack.hpp"
    #include "libdnf/common/sack/match_int64.hpp"
    #include "libdnf/common/sack/match_string.hpp"
    #include "libdnf/common/set.hpp"
%}

%ignore libdnf::Set::Set;
%ignore libdnf::sack::operator|(QueryCmp lhs, QueryCmp rhs);
%ignore libdnf::sack::operator&(QueryCmp lhs, QueryCmp rhs);
%include "libdnf/common/sack/query_cmp.hpp"
%include "libdnf/common/sack/query.hpp"
%include "libdnf/common/sack/sack.hpp"
%include "libdnf/common/sack/match_int64.hpp"
%include "libdnf/common/sack/match_string.hpp"
%include "libdnf/common/set.hpp"

%{
    #include "libdnf/common/preserve_order_map.hpp"
%}

#if defined(SWIGPYTHON) || defined(SWIGRUBY)
%extend libdnf::PreserveOrderMap {
    %fragment(SWIG_Traits_frag(libdnf::PreserveOrderMap<Key, T, KeyEqual>), "header", fragment="SwigPyIterator_T", fragment=SWIG_Traits_frag(T), fragment="LibdnfPreserveOrderMapTraits") {
        namespace swig {
            template <>  struct traits<libdnf::PreserveOrderMap<Key, T, KeyEqual>> {
                typedef pointer_category category;
                static const char* type_name() {
                    return "libdnf::PreserveOrderMap<" #Key "," #T "," #KeyEqual " >";
                }
            };
        }
    }

    %typemaps_asptr(SWIG_TYPECHECK_VECTOR, swig::asptr,
                    SWIG_Traits_frag(libdnf::PreserveOrderMap<Key, T, KeyEqual>),
                    libdnf::PreserveOrderMap<Key, T, KeyEqual>);

    inline bool __contains__(const Key & key) const { return $self->count(key) > 0; }

#if defined(SWIGPYTHON)
    %typemap(out,noblock=1) iterator, const_iterator {
      $result = SWIG_NewPointerObj(swig::make_output_iterator((const $type &)$1),
                                   swig::SwigPyIterator::descriptor(), SWIG_POINTER_OWN);
    }

    inline size_t __len__() const { return $self->size(); }

    inline const T & __getitem__(const Key & key) const { return $self->at(key); }

    inline void __setitem__(const Key & key, const T & v) { (*$self)[key] = v; }

    inline void __delitem__(const Key & key) {
        if ($self->erase(key) == 0) {
            throw std::out_of_range("PreserveOrderMap::__delitem__");
        }
    }

    %newobject __iter__(PyObject **PYTHON_SELF);
    swig::SwigPyIterator * __iter__(PyObject **PYTHON_SELF) {
        return swig::make_output_iterator(self->begin(), self->begin(), self->end(), *PYTHON_SELF);
    }

#elif defined(SWIGRUBY)

    inline bool __contains__(const Key & key) const { return $self->count(key) > 0; }

    inline size_t __len__() const { return $self->size(); }

    inline const T & __getitem__(const Key & key) const { return $self->at(key); }

    inline void __setitem__(const Key & key, const T & v) { (*$self)[key] = v; }

    inline void __delitem__(const Key & key) {
        if ($self->erase(key) == 0) {
            throw std::out_of_range("PreserveOrderMap::__delitem__");
        }
    }

    %newobject key_iterator(VALUE *RUBY_SELF);
    swig::ConstIterator* key_iterator(VALUE *RUBY_SELF) {
      return swig::make_output_key_iterator($self->begin(), $self->begin(),
                                            $self->end(), *RUBY_SELF);
    }

    %newobject value_iterator(VALUE *RUBY_SELF);
    swig::ConstIterator* value_iterator(VALUE *RUBY_SELF) {
      return swig::make_output_value_iterator($self->begin(), $self->begin(),
                                              $self->end(), *RUBY_SELF);
    }
#endif
}
#endif
#if defined(SWIGRUBY)
%rename("empty?") libdnf::PreserveOrderMap::empty;
%rename("include?") libdnf::PreserveOrderMap::__contains__ const;
#endif
%include "libdnf/common/preserve_order_map.hpp"

%template(PreserveOrderMapStringString) libdnf::PreserveOrderMap<std::string, std::string>;
%template(PreserveOrderMapStringPreserveOrderMapStringString) libdnf::PreserveOrderMap<std::string, libdnf::PreserveOrderMap<std::string, std::string>>;

%exception;  // beware this resets all exception handlers if you import this file after defining any
