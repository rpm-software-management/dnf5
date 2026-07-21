Name: foo
Version: 1.0.0
Release: 0%{?dist}
Summary: A utility that provides a binary named bar

License: GPLv3+

Provides: foo

%description
A utility that provides a binary named bar.

%install
mkdir -p %{buildroot}%{_bindir}
touch %{buildroot}%{_bindir}/bar

%files
%{_bindir}/bar


%changelog
