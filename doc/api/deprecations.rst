############
Deprecations
############

While the deprecations here are denoted in C++ they also include bindings versions of the API.

Currently deprecated methods:
=============================

.. doxygenpage:: deprecated
    :content-only:


📢 Deprecation signaling
========================

- build-time warnings using ``[[deprecated]]`` attribute, see: `cppreference <https://en.cppreference.com/w/cpp/language/attributes/deprecated>`_
- run-time warnings on ``stderr`` via ``LIBDNF5_DEPRECATED(msg)``
- deprecated API is marked in documentation using the ``@deprecated`` tag
- deprecations should be publicly announced on mailing list and GitHub
- deprecations should include guidance on what to use instead

Example:

.. code-block:: cpp
    :caption: foo.hpp

    /// @deprecated Use baz()
    [[deprecated("Use baz()")]]
    void foo();

.. code-block:: cpp
    :caption: foo.cpp

    #include "utils/deprecate.hpp"

    void foo() {
        LIBDNF5_DEPRECATED("Use baz()");
        ...
    }
