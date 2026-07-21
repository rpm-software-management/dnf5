Summary:        Updated test package for createrepo_c
Name:           package
Version:        0.3.1
Release:        1%{?dist}
License:        GPLv2+
URL:            https://github.com/rpm-software-management/package

Obsoletes:      package < 0.1.0
Provides:       package = %{version}-%{release}

%description
Test installable package for createrepo_c tests.
It is an updated version of ../createrepo_c-ci-packages/package-0.2.1-1.spec

%install

%files

%changelog
