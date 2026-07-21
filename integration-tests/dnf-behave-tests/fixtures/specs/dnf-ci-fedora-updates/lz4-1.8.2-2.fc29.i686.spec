%undefine _debuginfo_subpackages

Name:           lz4
Epoch:          0
Version:        1.8.2
Release:        2.fc29

License:        GPLv2+ and BSD
URL:            https://lz4.github.io/lz4/

Summary:        Extremely fast compression algorithm

Provides:       lz4 = 1.8.2-2.fc29
Provides:       lz4(x86-64) = 1.8.2-2.fc29

#Requires:       rtld(GNU_HASH)
#Requires:       libc.so.6(GLIBC_2.17)(64bit)

Obsoletes:      lz4 < 1.7.5-3

%description
LZ4 is an extremely fast loss-less compression algorithm, providing compression
speed at 400 MB/s per core, scalable with multi-core CPU. It also features
an extremely fast decoder, with speed in multiple GB/s per core, typically
reaching RAM speed limits on multi-core systems.

%package debuginfo
Summary:        Debug information for package lz4

Provides:       lz4-debuginfo = 1.8.2-2.fc29
Provides:       lz4-debuginfo(x86-64) = 1.8.2-2.fc29
Provides:       debuginfo(build-id) = 4ddde58b5b5caa4261fad46b45d791491a1f6aee

Recommends:     lz4-debugsource(x86-64) = 1.8.2-2.fc29

%description debuginfo
This package provides debug information for package lz4.
Debug information is useful when developing applications that use this
package or when debugging this package.

%package debugsource
Summary:        Debug sources for package lz4

Provides:       lz4-debugsource(x86-64) = 1.8.2-2.fc29
Provides:       lz4-debugsource = 1.8.2-2.fc29

%description debugsource
This package provides debug sources for package lz4.
Debug sources are useful when developing applications that use this
package or when debugging this package.

%package devel
Summary:        Development files for lz4

Provides:       lz4-devel = 1.8.2-2.fc29
Provides:       pkgconfig(liblz4) = 1.8.2
Provides:       lz4-devel(x86-64) = 1.8.2-2.fc29

#Requires:       /usr/bin/pkg-config
Requires:       liblz4.so.1()(64bit)
Requires:       lz4-libs(x86-64) = 1.8.2-2.fc29

%description devel
This package contains the header(.h) and library(.so) files required to build
applications using liblz4 library.

%package libs
Summary:        Libaries for lz4

Provides:       liblz4.so.1()(64bit)
Provides:       lz4-libs(x86-64) = 1.8.2-2.fc29
Provides:       lz4-libs = 1.8.2-2.fc29

#Requires:       rtld(GNU_HASH)
#Requires:       libc.so.6(GLIBC_2.14)(64bit)

Obsoletes:      lz4 < 1.7.5-3

%description libs
This package contains the libaries for lz4.

%package libs-debuginfo
Summary:        Debug information for package lz4-libs

Provides:       lz4-libs-debuginfo = 1.8.2-2.fc29
Provides:       debuginfo(build-id) = 731754aa7f68efc1c9c27f23a5dd1794ce27eb95
Provides:       lz4-libs-debuginfo(x86-64) = 1.8.2-2.fc29

Requires:       lz4-debuginfo(x86-64) = 1.8.2-2.fc29

Recommends:     lz4-debugsource(x86-64) = 1.8.2-2.fc29

%description libs-debuginfo
This package provides debug information for package lz4-libs.
Debug information is useful when developing applications that use this
package or when debugging this package.

%package static
Summary:        Static library for lz4

Provides:       lz4-static = 1.8.2-2.fc29
Provides:       lz4-static(x86-64) = 1.8.2-2.fc29

%description static
LZ4 is an extremely fast loss-less compression algorithm. This package
contains static libraries for static linking of applications.

%files

%files debuginfo

%files debugsource

%files devel

%files libs

%files libs-debuginfo

%files static

%changelog
