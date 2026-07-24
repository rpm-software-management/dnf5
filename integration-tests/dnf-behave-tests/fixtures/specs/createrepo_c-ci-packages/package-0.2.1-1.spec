Summary:        Test package for createrepo_c
Name:           package
Version:        0.2.1
Release:        1%{?dist}
License:        GPLv2+
URL:            https://github.com/rpm-software-management/package

Requires:       gcc
Obsoletes:      package < 0.1.0
Provides:       package = %{version}-%{release}

%description
Test un-installable package for createrepo_c tests.

%package libs
Summary:    Package libs

%description libs
Test library package

%package devel
Summary:    Devel package
Requires:   %{name}-libs%{?_isa} = %{version}-%{release}

%description devel
This is a devel package.

%package -n python3-%{name}
Summary:   Python 3 bindings for the package
%{?python_provide:%python_provide python3-%{name}}
Requires:  %{name}-libs = %{version}-%{release}

%description -n python3-%{name}
Python 3 bindings for the test package.

%package -n python2-%{name}
Summary:   Python bindings for the package
Requires:  python2-devel

%description -n python2-%{name}
Python bindings for the test package

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_mandir}/man8/
mkdir -p %{buildroot}%{_libdir}
touch %{buildroot}%{_bindir}/package
touch %{buildroot}%{_mandir}/man8/package.8
touch %{buildroot}%{_libdir}/lib%{name}.so
mkdir -p %{buildroot}%{_includedir}/%{name}/
mkdir -p %{buildroot}%{_libdir}/pkgconfig/
touch %{buildroot}%{_libdir}/pkgconfig/%{name}.pc
ln -sr %{buildroot}%{_bindir}/package %{buildroot}%{_bindir}/package_c
mkdir %{buildroot}/tmp

%files
%{_mandir}/man8/package.8*
%{_bindir}/package
%{_bindir}/package_c
%dir /
%dir /tmp

%files libs
%{_libdir}/lib%{name}.so

%files devel
%{_libdir}/lib%{name}.so
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/%{name}/

%files -n python2-%{name}

%files -n python3-%{name}

%changelog
