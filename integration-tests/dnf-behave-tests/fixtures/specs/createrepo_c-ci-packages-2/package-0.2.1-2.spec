Summary:        Duplicate test package for createrepo_c
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
It has the exact same NEVRA but different content (checksum) as ../createrepo_c-ci-packages/package-0.2.1-1.spec

%install

%files

%changelog
