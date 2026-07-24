%define _source_payload w0.zstdio
%define _binary_payload w0.zstdio

Summary:        Test package for createrepo_c
Name:           zstd-package
Version:        0.3.1
Release:        1%{?dist}
License:        GPLv2+
URL:            https://github.com/rpm-software-management/package

Provides:       zstd-package = %{version}-%{release}

%description
Test un-installable package with zstd compression

%install
head -c 10000 </dev/urandom >%{buildroot}/package_data

%files
/package_data

%changelog
