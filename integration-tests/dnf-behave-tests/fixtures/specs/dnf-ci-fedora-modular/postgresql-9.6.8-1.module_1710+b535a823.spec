%undefine _debuginfo_subpackages

Name:           postgresql
Epoch:          0
Version:        9.6.8
Release:        1.module_1710+b535a823

License:        PostgreSQL
URL:            http://www.postgresql.org/

Summary:        PostgreSQL client programs

Provides:       postgresql(x86-64) = 9.6.8-1.module_1710+b535a823
Provides:       postgresql = 9.6.8-1.module_1710+b535a823

Requires:       rtld(GNU_HASH)
Requires:       libpq.so.5()(64bit)
Requires:       postgresql-libs(x86-64) = 9.6.8-1.module_1710+b535a823

%description
PostgreSQL is an advanced Object-Relational database management system (DBMS).
The base postgresql package contains the client programs that you'll need to
access a PostgreSQL DBMS server, as well as HTML documentation for the whole
system.  These client programs can be located on the same machine as the
PostgreSQL server, or on a remote machine that accesses a PostgreSQL server
over a network connection.  The PostgreSQL server can be found in the
postgresql-server sub-package.

%package libs
Summary:        The shared libraries required for any PostgreSQL clients

Provides:       libpq.so.5()(64bit)
Provides:       libecpg.so.6()(64bit)
Provides:       libecpg_compat.so.3()(64bit)
Provides:       libpgtypes.so.3()(64bit)
Provides:       postgresql-libs(x86-64) = 9.6.8-1.module_1710+b535a823
Provides:       libpq.so = 9.6.8-1.module_1710+b535a823
Provides:       postgresql-libs = 9.6.8-1.module_1710+b535a823

Requires:       rtld(GNU_HASH)

%description libs
The postgresql-libs package provides the essential shared libraries for any
PostgreSQL client program or interface. You will need to install this package
to use any other PostgreSQL package or any clients that need to connect to a
PostgreSQL server.

%package server
Summary:        The programs needed to create and run a PostgreSQL server

Provides:       postgresql-server(:MODULE_COMPAT_9.6)
Provides:       postgresql-server(x86-64) = 9.6.8-1.module_1710+b535a823
Provides:       config(postgresql-server) = 9.6.8-1.module_1710+b535a823
Provides:       postgresql-server = 9.6.8-1.module_1710+b535a823

Requires:       rtld(GNU_HASH)
Requires:       libpq.so.5()(64bit)
Requires:       postgresql-libs(x86-64) = 9.6.8-1.module_1710+b535a823
Requires:       postgresql(x86-64) = 9.6.8-1.module_1710+b535a823

%description server
PostgreSQL is an advanced Object-Relational database management system (DBMS).
The postgresql-server package contains the programs needed to create
and run a PostgreSQL server, which will in turn allow you to create
and maintain PostgreSQL databases.

%package test
Summary:        The test suite distributed with PostgreSQL

Provides:       postgresql-test = 9.6.8-1.module_1710+b535a823
Provides:       postgresql-test(x86-64) = 9.6.8-1.module_1710+b535a823

Requires:       postgresql-server(x86-64) = 9.6.8-1.module_1710+b535a823

%description test
The postgresql-test package contains files needed for various tests for the
PostgreSQL database management system, including regression tests and
benchmarks.

%files

%files libs

%files server

%files test

%changelog
