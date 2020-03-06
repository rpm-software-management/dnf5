Coding style
============

Character case:

* Types: CamelCase
* Classes: CamelCase
* Functions: snake_case
* Variables: snake_case
* Arguments: snake_case
* Constants: UPPER_CASE


File system hierarchy:

* Directories: lower_case, avoid separators if possible
* Files: snake_case.{cpp,hpp}


C++ coding style:

* Use clang-format and clang-tidy to check your coding style before you submit a pull request
* -std=c++17
* Indent by 4 spaces
* 120 characters per line
* Includes grouped and alphabetically ordered within each group

  * project
  * project - "libdnf-cli/" absolute paths
  * project - "libdnf/" absolute paths
  * 3rd party
  * standard library

* Project includes must use full path in the project or plain file name (includes in the same directory)
* Use C++ style comments: ``//``
* Use three forward slashes for docstrings: ``///``
* Use templates and lambdas only if really necessary
* Use ``// clang-format off|on`` if auto-formatted code is not readable
* See .clang-format for more details and examples


Python coding style:

* 120 characters per line
* Follow pep8


Project layout
==============
```
  <top directory>
    +-- bindings                # everything related to SWIG bindings goes here
    |   +-- <language>          # go, perl5, python3, ruby
    |   |   +-- libdnf          # language specific bindings for libdnf
    |   |   +-- libdnf_cli      # language specific bindings for libdnf-cli
    |   +-- libdnf              # SWIG *.i files for libdnf
    |   +-- libdnf_cli          # SWIG *.i files for libdnf-cli
    +-- doc                     # documentation
    +-- include                 # public headers
    |   +-- libdnf              # libdnf public C++ headers
    |   +-- libdnf-cli          # libdnf-cli public C++ headers
    +-- libdnf                  # libdnf code and private headers (core libdnf functionality)
    |   +-- <module>            #
    +-- libdnf-cli              # libdnf-cli code and private headers (argument parser, progressbars, tables, ...)
    |   +-- <module>            #
    +-- dnfdaemon-client        # command line client for dnfdaemon-server
    +-- dnfdaemon-server        # DBus package manager service
    +-- microdnf                # microdnf command line package manager
    +-- test                    # tests; similar layout to the bindings
```
