%undefine _debuginfo_subpackages

Name:           lz4
Epoch:          0
Version:        1.7.5
Release:        2.fc26

License:        GPLv2+ and BSD
URL:            https://lz4.github.io/lz4/

Summary:        Extremely fast compression algorithm

Provides:       liblz4.so.1()(64bit)
Provides:       lz4(x86-64) = 1.7.5-2.fc26
Provides:       lz4 = 1.7.5-2.fc26

#Requires:       libc.so.6(GLIBC_2.14)(64bit)
#Requires:       rtld(GNU_HASH)
#Requires:       /sbin/ldconfig

%description
LZ4 is an extremely fast loss-less compression algorithm, providing compression
speed at 400 MB/s per core, scalable with multi-core CPU. It also features
an extremely fast decoder, with speed in multiple GB/s per core, typically
reaching RAM speed limits on multi-core systems.

%package debuginfo
Summary:        Debug information for package lz4

Provides:       lz4-debuginfo = 1.7.5-2.fc26
Provides:       lz4-debuginfo(x86-64) = 1.7.5-2.fc26

%description debuginfo
This package provides debug information for package lz4.
Debug information is useful when developing applications that use this
package or when debugging this package.

%package devel
Summary:        Development files for lz4

Provides:       pkgconfig(liblz4) = 1.7.5
Provides:       lz4-devel = 1.7.5-2.fc26
Provides:       lz4-devel(x86-64) = 1.7.5-2.fc26

#Requires:       /usr/bin/pkg-config
Requires:       liblz4.so.1()(64bit)
Requires:       lz4(x86-64) = 1.7.5-2.fc26

%description devel
This package contains the header(.h) and library(.so) files required to build
applications using liblz4 library.

%package static
Summary:        Static library for lz4

Provides:       lz4-static = 1.7.5-2.fc26
Provides:       lz4-static(x86-64) = 1.7.5-2.fc26

%description static
LZ4 is an extremely fast loss-less compression algorithm. This package
contains static libraries for static linking of applications.

%files

%files debuginfo

%files devel

%files static

%changelog
