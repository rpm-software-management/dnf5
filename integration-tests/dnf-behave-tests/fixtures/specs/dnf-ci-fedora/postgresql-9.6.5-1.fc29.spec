%undefine _debuginfo_subpackages

Name:           postgresql
Epoch:          0
Version:        9.6.5
Release:        1.fc29

License:        PostgreSQL
URL:            http://www.postgresql.org/

Summary:        PostgreSQL client programs

Provides:       postgresql(x86-64) = 9.6.5-1.fc29
Provides:       postgresql = 9.6.5-1.fc29

Requires:       rtld(GNU_HASH)
Requires:       libpq.so.5()(64bit)
Requires:       postgresql-libs(x86-64) = 9.6.5-1.fc29

%description
PostgreSQL is an advanced Object-Relational database management system (DBMS).
The base postgresql package contains the client programs that you'll need to
access a PostgreSQL DBMS server, as well as HTML documentation for the whole
system.  These client programs can be located on the same machine as the
PostgreSQL server, or on a remote machine that accesses a PostgreSQL server
over a network connection.  The PostgreSQL server can be found in the
postgresql-server sub-package.

%package contrib
Summary:        Extension modules distributed with PostgreSQL

Provides:       postgresql-contrib = 9.6.5-1.fc29
Provides:       postgresql-contrib(x86-64) = 9.6.5-1.fc29

Requires:       rtld(GNU_HASH)
Requires:       postgresql-libs(x86-64) = 9.6.5-1.fc29
Requires:       postgresql(x86-64) = 9.6.5-1.fc29

%description contrib
The postgresql-contrib package contains various extension modules that are
included in the PostgreSQL distribution.

%package devel
Summary:        PostgreSQL development header files and libraries

Provides:       postgresql-devel(x86-64) = 9.6.5-1.fc29
Provides:       pkgconfig(libecpg) = 9.6.5
Provides:       pkgconfig(libecpg_compat) = 9.6.5
Provides:       pkgconfig(libpgtypes) = 9.6.5
Provides:       pkgconfig(libpq) = 9.6.5
Provides:       postgresql-devel = 9.6.5-1.fc29

Requires:       rtld(GNU_HASH)
Requires:       libpq.so.5()(64bit)
Requires:       libpgtypes.so.3()(64bit)
Requires:       postgresql-libs(x86-64) = 9.6.5-1.fc29

%description devel
The postgresql-devel package contains the header files and libraries
needed to compile C or C++ applications which will directly interact
with a PostgreSQL database management server.  It also contains the ecpg
Embedded C Postgres preprocessor. You need to install this package if you want
to develop applications which will interact with a PostgreSQL server.

%package docs
Summary:        Extra documentation for PostgreSQL

Provides:       postgresql-doc = 9.6.5-1.fc29
Provides:       postgresql-docs = 9.6.5-1.fc29
Provides:       postgresql-docs(x86-64) = 9.6.5-1.fc29

Requires:       rtld(GNU_HASH)
Requires:       libc.so.6(GLIBC_2.14)(64bit)
Requires:       postgresql(x86-64) = 9.6.5-1.fc29

%description docs
The postgresql-docs package contains some additional documentation for
PostgreSQL.  Currently, this includes the main documentation in PDF format
and source files for the PostgreSQL tutorial.

%package libs
Summary:        The shared libraries required for any PostgreSQL clients

Provides:       libpq.so.5()(64bit)
Provides:       libecpg.so.6()(64bit)
Provides:       libecpg_compat.so.3()(64bit)
Provides:       libpgtypes.so.3()(64bit)
Provides:       postgresql-libs(x86-64) = 9.6.5-1.fc29
Provides:       libpq.so = 9.6.5-1.fc29
Provides:       postgresql-libs = 9.6.5-1.fc29

Requires:       rtld(GNU_HASH)

%description libs
The postgresql-libs package provides the essential shared libraries for any
PostgreSQL client program or interface. You will need to install this package
to use any other PostgreSQL package or any clients that need to connect to a
PostgreSQL server.

%package plperl
Summary:        The Perl procedural language for PostgreSQL

Provides:       postgresql-plperl = 9.6.5-1.fc29
Provides:       postgresql-plperl(x86-64) = 9.6.5-1.fc29

Requires:       postgresql-server(x86-64) = 9.6.5-1.fc29

%description plperl
The postgresql-plperl package contains the PL/Perl procedural language,
which is an extension to the PostgreSQL database server.
Install this if you want to write database functions in Perl.

%package plpython
Summary:        The Python2 procedural language for PostgreSQL

Provides:       postgresql-plpython = 9.6.5-1.fc29
Provides:       postgresql-plpython(x86-64) = 9.6.5-1.fc29

Requires:       postgresql-server(x86-64) = 9.6.5-1.fc29

%description plpython
The postgresql-plpython package contains the PL/Python procedural language,
which is an extension to the PostgreSQL database server.
Install this if you want to write database functions in Python 2.

%package plpython3
Summary:        The Python3 procedural language for PostgreSQL

Provides:       postgresql-plpython3 = 9.6.5-1.fc29
Provides:       postgresql-plpython3(x86-64) = 9.6.5-1.fc29

Requires:       postgresql-server(x86-64) = 9.6.5-1.fc29

%description plpython3
The postgresql-plpython3 package contains the PL/Python3 procedural language,
which is an extension to the PostgreSQL database server.
Install this if you want to write database functions in Python 3.

%package pltcl
Summary:        The Tcl procedural language for PostgreSQL

Provides:       postgresql-pltcl = 9.6.5-1.fc29
Provides:       postgresql-pltcl(x86-64) = 9.6.5-1.fc29

Requires:       postgresql-server(x86-64) = 9.6.5-1.fc29

%description pltcl
The postgresql-pltcl package contains the PL/Tcl procedural language,
which is an extension to the PostgreSQL database server.
Install this if you want to write database functions in Tcl.

%package server
Summary:        The programs needed to create and run a PostgreSQL server

Provides:       postgresql-server(:MODULE_COMPAT_9.6)
Provides:       postgresql-server(x86-64) = 9.6.5-1.fc29
Provides:       config(postgresql-server) = 9.6.5-1.fc29
Provides:       postgresql-server = 9.6.5-1.fc29

Requires:       rtld(GNU_HASH)
Requires:       libpq.so.5()(64bit)
Requires:       postgresql-libs(x86-64) = 9.6.5-1.fc29
Requires:       postgresql(x86-64) = 9.6.5-1.fc29

%description server
PostgreSQL is an advanced Object-Relational database management system (DBMS).
The postgresql-server package contains the programs needed to create
and run a PostgreSQL server, which will in turn allow you to create
and maintain PostgreSQL databases.

%package static
Summary:        Statically linked PostgreSQL libraries

Provides:       postgresql-static = 9.6.5-1.fc29
Provides:       postgresql-static(x86-64) = 9.6.5-1.fc29

Requires:       postgresql-devel(x86-64) = 9.6.5-1.fc29

%description static
Statically linked PostgreSQL libraries that do not have dynamically linked
counterparts.

%package test
Summary:        The test suite distributed with PostgreSQL

Provides:       postgresql-test = 9.6.5-1.fc29
Provides:       postgresql-test(x86-64) = 9.6.5-1.fc29

Requires:       postgresql-server(x86-64) = 9.6.5-1.fc29
Requires:       postgresql-devel(x86-64) = 9.6.5-1.fc29

%description test
The postgresql-test package contains files needed for various tests for the
PostgreSQL database management system, including regression tests and
benchmarks.

%package upgrade
Summary:        Support for upgrading from the previous major release of PostgreSQL

Provides:       postgresql-upgrade = 9.6.5-1.fc29
Provides:       postgresql-upgrade(x86-64) = 9.6.5-1.fc29

Requires:       postgresql-server(x86-64) = 9.6.5-1.fc29
Requires:       postgresql-libs(x86-64) = 9.6.5-1.fc29

%description upgrade
The postgresql-upgrade package contains the pg_upgrade utility and supporting
files needed for upgrading a PostgreSQL database from the previous major
version of PostgreSQL.

%files

%files contrib

%files devel

%files docs

%files libs

%files plperl

%files plpython

%files plpython3

%files pltcl

%files server

%files static

%files test

%files upgrade

%changelog
