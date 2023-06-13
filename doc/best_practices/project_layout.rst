Project layout
==============

* Directories: lower_case, avoid separators if possible
* Files: snake_case.{cpp,hpp,py,...}
* Project layout::

    <top directory>
    +-- bindings                # everything related to SWIG bindings goes here
    |   +-- <language>          # go, perl5, python3, ruby
    |   |   +-- libdnf5          # language specific bindings for libdnf5
    |   |   +-- libdnf5_cli      # language specific bindings for libdnf5-cli
    |   +-- libdnf5              # SWIG *.i files for libdnf5
    |   +-- libdnf5_cli          # SWIG *.i files for libdnf5-cli
    +-- doc                     # documentation
    +-- include                 # public headers
    |   +-- libdnf5              # libdnf5 public C++ headers
    |   +-- libdnf5-cli          # libdnf5-cli public C++ headers
    +-- libdnf5                  # libdnf5 code and private headers
    +-- libdnf5-cli              # libdnf5-cli code and private headers
    +-- libdnf5-plugins          # libdnf5 C/C++ plugins
    +-- dnf5daemon-client       # command line client for dnf5daemon-server
    +-- dnf5daemon-server       # DBus package manager service
    +-- dnf5                    # dnf5 command line package manager
    +-- dnf5-plugins            # dnf5 plugins
    +-- test                    # tests; similar layout to the bindings
