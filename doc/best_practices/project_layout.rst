Project layout
==============

* Directories: lower_case, avoid separators if possible
* Files: snake_case.{cpp,hpp,py,...}
* Project layout::

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
    +-- libdnf                  # libdnf code and private headers
    +-- libdnf-cli              # libdnf-cli code and private headers
    +-- libdnf-plugins          # libdnf C/C++ plugins
    +-- dnf5daemon-client       # command line client for dnf5daemon-server
    +-- dnf5daemon-server       # DBus package manager service
    +-- dnf5                    # dnf5 command line package manager
    +-- dnf5-plugins            # dnf5 plugins
    +-- test                    # tests; similar layout to the bindings
