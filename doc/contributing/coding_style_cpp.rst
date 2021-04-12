C++ coding style
================

* C++17
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

    #include <libdnf-cli/.../*.hpp>

    #include <libdnf/.../*.hpp>

    #include <3rd party>

    #include <standard library>

* Includes within the same directory should use relative paths::

     #include "current-dir-include.hpp"

* Other includes should use absolute paths::

    #include <libdnf/.../*.hpp>
