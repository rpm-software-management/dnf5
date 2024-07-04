Symbol visibility
=================

The DNF5 build is configured so that the symbols in the generated dynamic libraries are hidden by default - they are not exported.
Therefore, all non-inline public functions and classes (structures) must be explicitly marked for export.
If a class (struct) is marked for export, all its symbols are automatically exported. For this reason,
private members of public classes (structures) must be explicitly marked as local to avoid exporting their symbols.
We do not want to expose private symbols in the ABI.

The public API is defined in public header files. It is the only place where symbols are marked for export or as local
in the case of private members.

To mark whether a symbol should be exported, we use macros. The macros are defined in the `defs.h` files.
The different parts of the project have their own specific macros. This is because the public header files are shared.
For example, the libdnf5 header file marks the symbol for export when compiling the libdnf5 library, but the same symbol
for import when including that header file in the libdnf5-cli library.

Macros used
-----------

* ``LIBDNF_API``, ``LIBDNF_LOCAL``, ``LIBDNF_PLUGIN_API``
* ``LIBDNF_CLI_API``, ``LIBDNF_CLI_LOCAL``
* ``DNF_API``, ``DNF_LOCAL``

| Macros ending with ``_API`` denote symbols for export.
| Macros ending with ``_LOCAL`` denote private symbols and are used for private members of public classes (structures).

Example
-------

::

    #include "libdnf5/defs.h"

    class LIBDNF_API PublicExampleClass {
    public:
        void public_method1();
        int public_method2();

    private:
        LIBDNF_LOCAL void private_method();

        class LIBDNF_LOCAL Impl;  // Impl is private class

        ImplPtr<Impl> p_impl;
    }

    LIBDNF_API void public_function();
