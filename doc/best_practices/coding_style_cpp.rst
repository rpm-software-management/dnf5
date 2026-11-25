C++ coding style
================

* C++20
* Indent by 4 spaces
* Up to 120 characters per line
* Comments start with two forward slashes: ``//``
* Docstrings start with three forward slashes: ``///``
* See .clang-format for more details and examples


Character case
--------------

* Types: CamelCase
* Classes: CamelCase
* Functions: snake_case
* Variables: snake_case
* Arguments: snake_case
* Constants: UPPER_CASE


Includes
--------
* Includes grouped and alphabetically ordered within each group::

    #include "current-dir-include.hpp"

    #include <libdnf5-cli/.../*.hpp>

    #include <libdnf5/.../*.hpp>

    #include <3rd party>

    #include <standard library>

* Includes within the same directory should use relative paths::

     #include "current-dir-include.hpp"

* Other includes should use absolute paths::

    #include <libdnf5/.../*.hpp>

* Includes within the same component should use the double-quote form. E.g. in dnf5::

    #include "commands/.../*.hpp"

* Includes from a different component in the repository should use the angle-bracket form. E.g. in dnf5::

    #include <libdnf5/.../*.hpp>


Exceptions
----------

* Never let an exception escape across a C boundary. Functions passed as C
  callbacks (e.g. to librpm or librepo) and invoked from C code must catch every
  exception, because unwinding through C frames is undefined behavior and can,
  for an rpm transaction, leave the system in an undefined state.

* In such a callback, catch with ``catch (...)``, log the error, and translate it
  into a value the C API understands (for example a librepo error return code).
  When the exception must still reach the caller, store it with
  ``std::current_exception()`` and rethrow it with ``std::rethrow_exception()``
  from a C++ frame once the C call has safely returned, rather than letting it
  propagate through the C boundary.

* Destructors are implicitly ``noexcept``: a callback invoked from a destructor
  must also swallow any exception, or ``std::terminate()`` is called during stack
  unwinding.
