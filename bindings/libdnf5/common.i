#if defined(SWIGPYTHON)
%module(package="libdnf5") common
#elif defined(SWIGPERL)
%module "libdnf5::common"
#elif defined(SWIGRUBY)
%module "libdnf5::common"
#endif

%include <cstring.i>
%include <exception.i>
%include <stdint.i>
%include <std_map.i>
%include <std_pair.i>
#if defined(SWIGPYTHON) || defined(SWIGRUBY)
%include <std_set.i>
#endif
%include <std_string.i>
%include <std_vector.i>

%include "shared.i"

%import "exception.i"

%typemap(out) std::string * {
    if ($1 == nullptr) {
        $result = SWIG_FromCharPtrAndSize("", 0);
    } else {
        $result = SWIG_FromCharPtrAndSize($1->c_str(), $1->size());
    }
}

%{
    #include "bindings/libdnf5/exception.hpp"

    #include "libdnf5/common/message.hpp"
    #include "libdnf5/common/weak_ptr.hpp"
%}

// Deletes any previously defined general purpose exception handler
%exception;

// Set default exception handler
%catches(libdnf5::UserAssertionError, std::runtime_error, std::out_of_range);

%include "libdnf5/common/message.hpp"

%include "libdnf5/common/weak_ptr.hpp"
#if defined(SWIGPYTHON)
%extend libdnf5::WeakPtr {
    intptr_t __hash__() const {
        return reinterpret_cast<intptr_t>($self->get());
    }
}
#endif

%ignore std::vector::vector(size_type);
%ignore std::vector::vector(unsigned int);
%ignore std::vector::resize;

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

%define fix_swigtype_trait(ClassName)
%traits_swigtype(ClassName)
%fragment(SWIG_Traits_frag(ClassName));
%enddef

%define add_ruby_each(ClassName)
#if defined(SWIGRUBY)
/* Using swig::from method on a class instance (= $self in %extend blocks below) fails.
 * We need to define these traits so that it compiles. This seems to be an issue related
 * to namespacing:
 * https://github.com/swig/swig/issues/973
 * and to using a pointer in some places:
 * https://github.com/swig/swig/issues/2938
 */
fix_swigtype_trait(ClassName)

%extend ClassName {
    VALUE each() {
        // If no block is provided, returns Enumerator.
        RETURN_ENUMERATOR(swig::from($self), 0, 0);

        VALUE r;
        auto i = self->begin();
        auto e = self->end();

        for (; i != e; ++i) {
            r = swig::from(*i);
            rb_yield(r);
        }

        return swig::from($self);
    }
}
#endif
%enddef

%define add_str(ClassName)
#if defined(SWIGPYTHON)
%extend ClassName {
    std::string __str__() const {
        return $self->to_string();
    }
}
#endif
%enddef

%define add_repr(ClassName)
#if defined(SWIGPYTHON)
%extend ClassName {
    std::string __repr__() const {
        return $self->to_string_description();
    }
}
#endif
%enddef

%define add_hash(ClassName)
#if defined(SWIGPYTHON)
%extend ClassName {
    int __hash__() const {
        return $self->get_hash();
    }
}
#endif
%enddef

%{
    #include "libdnf5/common/sack/query.hpp"
    #include "libdnf5/common/sack/query_cmp.hpp"
    #include "libdnf5/common/sack/sack.hpp"
    #include "libdnf5/common/sack/match_int64.hpp"
    #include "libdnf5/common/sack/match_string.hpp"
    #include "libdnf5/common/set.hpp"
%}

%ignore libdnf5::Set::Set;
%ignore libdnf5::sack::operator|(QueryCmp lhs, QueryCmp rhs);
%ignore libdnf5::sack::operator&(QueryCmp lhs, QueryCmp rhs);
%include "libdnf5/common/sack/query_cmp.hpp"
%include "libdnf5/common/sack/query.hpp"
%include "libdnf5/common/sack/sack.hpp"
%include "libdnf5/common/sack/match_int64.hpp"
%include "libdnf5/common/sack/match_string.hpp"

%rename(next) libdnf5::SetConstIterator::operator++();
%rename(prev) libdnf5::SetConstIterator::operator--();
%rename(value) libdnf5::SetConstIterator::operator*() const;
%ignore libdnf5::SetConstIterator::operator++(int);
%ignore libdnf5::SetConstIterator::operator--(int);
%ignore libdnf5::SetConstIterator::operator->() const;

%typemap(out, noblock=1) copy_return_value {
    // create a copy that is owned by the caller
    $result = SWIG_NewPointerObj((new $*1_ltype(*$1)), $1_descriptor, 1);
}

%include "libdnf5/common/set.hpp"
#if defined(SWIGPYTHON)
%extend libdnf5::Set {
    size_type __len__() const noexcept {
        return $self->size();
    }
}
#endif

%{
    #include "libdnf5/common/preserve_order_map.hpp"
%}

#if defined(SWIGPYTHON) || defined(SWIGRUBY)
%extend libdnf5::PreserveOrderMap {
    %fragment(SWIG_Traits_frag(libdnf5::PreserveOrderMap<Key, T, KeyEqual>), "header", fragment="SwigPyIterator_T", fragment=SWIG_Traits_frag(T), fragment="LibdnfPreserveOrderMapTraits") {
        namespace swig {
            template <>  struct traits<libdnf5::PreserveOrderMap<Key, T, KeyEqual>> {
                typedef pointer_category category;
                static const char* type_name() {
                    return "libdnf5::PreserveOrderMap<" #Key "," #T "," #KeyEqual " >";
                }
            };
        }
    }

    %typemaps_asptr(SWIG_TYPECHECK_VECTOR, swig::asptr,
                    SWIG_Traits_frag(libdnf5::PreserveOrderMap<Key, T, KeyEqual>),
                    libdnf5::PreserveOrderMap<Key, T, KeyEqual>);

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
%rename("empty?") libdnf5::PreserveOrderMap::empty;
%rename("include?") libdnf5::PreserveOrderMap::__contains__ const;
#endif
%include "libdnf5/common/preserve_order_map.hpp"

%template(PreserveOrderMapStringString) libdnf5::PreserveOrderMap<std::string, std::string>;
%template(PreserveOrderMapStringPreserveOrderMapStringString) libdnf5::PreserveOrderMap<std::string, libdnf5::PreserveOrderMap<std::string, std::string>>;

// The following adds Python attribute shortcuts for getters and setters
// from C++ structures that act as plain data objects.
//
// E.g. object.get_value() -> object.value
//
#if defined(SWIGPYTHON)
%pythoncode %{
def create_attributes_from_getters_and_setters(cls):
    getter_prefix = 'get_'
    setter_prefix = 'set_'
    attrs = {method[len(getter_prefix):] for method in dir(cls) if method.startswith(getter_prefix) or method.startswith(setter_prefix)}
    for attr in attrs:
        getter_name = getter_prefix + attr
        setter_name = setter_prefix + attr
        setattr(cls, attr, property(
            lambda self, getter_name=getter_name: getattr(self, getter_name)() if getter_name in dir(cls) else None,
            lambda self, value, setter_name=setter_name: getattr(self, setter_name)(value) if setter_name in dir(cls) else None
        ))
%}
#endif

// Base weak ptr is used across the codebase
%include "libdnf5/base/base_weak.hpp"

// Deletes any previously defined catches
%catches();
