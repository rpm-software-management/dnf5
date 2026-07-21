%undefine _debuginfo_subpackages

Name:           zstd
Epoch:          0
Version:        1.3.6
Release:        1.fc29

License:        BSD and GPLv2
URL:            https://github.com/facebook/zstd

Summary:        Zstd compression library

Provides:       zstd = 1.3.6-1.fc29
Provides:       zstd(x86-64) = 1.3.6-1.fc29

Requires:       rtld(GNU_HASH)
Requires:       libm.so.6()(64bit)
Requires:       libpthread.so.0()(64bit)
Requires:       libgcc_s.so.1()(64bit)
Requires:       libstdc++.so.6()(64bit)
Requires:       libstdc++.so.6(CXXABI_1.3)(64bit)
Requires:       libstdc++.so.6(GLIBCXX_3.4)(64bit)
Requires:       libpthread.so.0(GLIBC_2.2.5)(64bit)
Requires:       libgcc_s.so.1(GCC_3.0)(64bit)
Requires:       libstdc++.so.6(GLIBCXX_3.4.21)(64bit)
Requires:       libstdc++.so.6(GLIBCXX_3.4.11)(64bit)
Requires:       libstdc++.so.6(GLIBCXX_3.4.20)(64bit)
Requires:       libpthread.so.0(GLIBC_2.3.2)(64bit)
Requires:       libstdc++.so.6(GLIBCXX_3.4.14)(64bit)
Requires:       libc.so.6(GLIBC_2.17)(64bit)
Requires:       libstdc++.so.6(GLIBCXX_3.4.19)(64bit)
Requires:       libstdc++.so.6(GLIBCXX_3.4.22)(64bit)
Requires:       libstdc++.so.6(GLIBCXX_3.4.17)(64bit)

%description
Zstd, short for Zstandard, is a fast lossless compression algorithm,
targeting real-time compression scenarios at zlib-level compression ratio.

%package -n libzstd
Summary:        Zstd shared library

Provides:       libzstd.so.1()(64bit)
Provides:       libzstd = 1.3.6-1.fc29
Provides:       libzstd(x86-64) = 1.3.6-1.fc29

Requires:       rtld(GNU_HASH)
Requires:       libc.so.6(GLIBC_2.14)(64bit)

%description -n libzstd
Zstandard compression shared library.

%package -n libzstd-debuginfo
Summary:        Debug information for package libzstd

Provides:       libzstd-debuginfo = 1.3.6-1.fc29
Provides:       debuginfo(build-id) = fe5a1e1997a9c3876cff2431d35eb32f43093d0a
Provides:       libzstd-debuginfo(x86-64) = 1.3.6-1.fc29

Requires:       zstd-debuginfo(x86-64) = 1.3.6-1.fc29

Recommends:     zstd-debugsource(x86-64) = 1.3.6-1.fc29

%description -n libzstd-debuginfo
This package provides debug information for package libzstd.
Debug information is useful when developing applications that use this
package or when debugging this package.

%package -n libzstd-devel
Summary:        Header files for Zstd library

Provides:       libzstd-devel = 1.3.6-1.fc29
Provides:       pkgconfig(libzstd) = 1.3.6
Provides:       libzstd-devel(x86-64) = 1.3.6-1.fc29

Requires:       /usr/bin/pkg-config
Requires:       libzstd.so.1()(64bit)
Requires:       libzstd(x86-64) = 1.3.6-1.fc29

%description -n libzstd-devel
Header files for Zstd library.

%package debuginfo
Summary:        Debug information for package zstd

Provides:       zstd-debuginfo(x86-64) = 1.3.6-1.fc29
Provides:       zstd-debuginfo = 1.3.6-1.fc29
Provides:       debuginfo(build-id) = 0da946aad525ae0642116b1e9da136840d9dbfe3
Provides:       debuginfo(build-id) = dfca8180cfe5d3e3ca95d63634314198a05eeca3

Recommends:     zstd-debugsource(x86-64) = 1.3.6-1.fc29

%description debuginfo
This package provides debug information for package zstd.
Debug information is useful when developing applications that use this
package or when debugging this package.

%package debugsource
Summary:        Debug sources for package zstd

Provides:       zstd-debugsource(x86-64) = 1.3.6-1.fc29
Provides:       zstd-debugsource = 1.3.6-1.fc29

%description debugsource
This package provides debug sources for package zstd.
Debug sources are useful when developing applications that use this
package or when debugging this package.

%files -n libzstd

%files -n libzstd-debuginfo

%files -n libzstd-devel

%files

%files debuginfo

%files debugsource

%changelog
