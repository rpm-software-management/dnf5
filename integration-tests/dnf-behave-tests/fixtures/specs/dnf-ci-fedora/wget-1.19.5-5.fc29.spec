Name: wget
Version: 1.19.5
Release: 5%{?dist}
Summary: A utility for retrieving files using the HTTP or FTP protocols

License: GPLv3+
Group: Applications/Internet
Url: http://www.gnu.org/software/wget/

Provides: webclient
Provides: bundled(gnulib)

%description
GNU Wget is a file retrieval utility which can use either the HTTP or
FTP protocols. Wget features include the ability to work in the
background while you are logged out, recursive retrieval of
directories, file name wildcard matching, remote file timestamp
storage and comparison, use of Rest with FTP servers and Range with
HTTP servers to retrieve files over slow or unstable connections,
support for Proxy servers, and configurability.

%install
mkdir -p %{buildroot}%{_bindir}
touch %{buildroot}%{_bindir}/wget-binary

%files
%{_bindir}/wget-binary

%changelog
