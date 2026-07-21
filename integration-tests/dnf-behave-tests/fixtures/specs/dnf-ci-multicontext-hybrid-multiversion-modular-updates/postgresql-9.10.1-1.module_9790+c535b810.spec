%undefine _debuginfo_subpackages

Name:           postgresql
Epoch:          0
Version:        9.10.1
Release:        1.module_9790+c535b810

License:        PostgreSQL
URL:            http://www.postgresql.org/

Summary:        PostgreSQL client programs

Provides:       postgresql(x86-64) = 9.10.1-1.module_9790+c535b810
Provides:       postgresql = 9.10.1-1.module_9790+c535b810


%description
PostgreSQL is an advanced Object-Relational database management system (DBMS).
The base postgresql package contains the client programs that you'll need to
access a PostgreSQL DBMS server, as well as HTML documentation for the whole
system.  These client programs can be located on the same machine as the
PostgreSQL server, or on a remote machine that accesses a PostgreSQL server
over a network connection.  The PostgreSQL server can be found in the
postgresql-server sub-package.

%files

%changelog
