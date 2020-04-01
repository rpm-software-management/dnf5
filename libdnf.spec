Name:           libdnf
Version:        1.0.0
Release:        1%{?dist}
Summary:        Package management library
License:        LGPLv2.1+
URL:            https://github.com/rpm-software-management/libdnf
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz


# ========== build options ==========

%bcond_without dnfdaemon_client
%bcond_without dnfdaemon_server
%bcond_without libdnf_cli
%bcond_without microdnf

%bcond_without comps
%bcond_without modulemd
%bcond_without zchunk

%bcond_with    html
%bcond_without man

%bcond_without go
%bcond_without perl5
%bcond_without python3
%bcond_without ruby

%bcond_with    sanitizers
%bcond_without tests
%bcond_with    performance_tests


# ========== versions of dependencies ==========

%global libmodulemd_version 2.5.0
%global librepo_version 1.11.0
%global libsolv_version 0.7.7
%global swig_version 3.0.12
%global zchunk_version 0.9.11


# ========== build requires ==========

BuildRequires:  cmake
BuildRequires:  doxygen
BuildRequires:  gcc-c++
BuildRequires:  gettext

BuildRequires:  pkgconfig(check)
%if ! %{with tests_disabled}
BuildRequires:  pkgconfig(cppunit)
%endif
BuildRequires:  pkgconfig(gpgme)
BuildRequires:  pkgconfig(json-c)
%if %{with comps}
BuildRequires:  pkgconfig(libcomps)
%endif
BuildRequires:  pkgconfig(libcrypto)
BuildRequires:  pkgconfig(librepo) >= %{librepo_version}
BuildRequires:  pkgconfig(libsolv) >= %{libsolv_version}
BuildRequires:  pkgconfig(libsolvext) >= %{libsolv_version}
%if %{with modulemd}
BuildRequires:  pkgconfig(modulemd-2.0) >= %{libmodulemd_version}
%endif
BuildRequires:  pkgconfig(rpm) >= 4.11.0
BuildRequires:  pkgconfig(sqlite3)
%if %{with zchunk}
BuildRequires:  pkgconfig(zck) >= %{zchunk_version}
%endif

%if %{with html} || %{with man}
BuildRequires:  python3dist(breathe)
BuildRequires:  python3dist(sphinx)
BuildRequires:  python3dist(sphinx-rtd-theme)
%endif

%if %{with sanitizers}
BuildRequires:  libasan-static
BuildRequires:  liblsan-static
BuildRequires:  libubsan-static
%endif


# ========== libdnf ==========
#Requires:       libmodulemd{?_isa} >= {libmodulemd_version}
Requires:       libsolv%{?_isa} >= %{libsolv_version}
Requires:       librepo%{?_isa} >= %{librepo_version}

%description
Package management library

%files
%{_libdir}/libdnf.so.*
%license lgpl-2.1.txt


# ========== libdnf-cli ==========

%if %{with libdnf_cli}
%package -n libdnf-cli
Summary:        Library for working with a terminal in a command-line package manager
BuildRequires:  pkgconfig(smartcols)

%description -n libdnf-cli
Library for working with a terminal in a command-line package manager.

%files -n libdnf-cli
%{_libdir}/libdnf-cli.so.*
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== libdnf-devel ==========

%package devel
Summary:        Development files for libdnf
Requires:       libdnf%{?_isa} = %{version}-%{release}
Requires:       libsolv-devel%{?_isa} >= %{libsolv_version}

%description devel
Development files for libdnf.

%files devel
%{_includedir}/libdnf/
%{_libdir}/libdnf.so
%{_libdir}/pkgconfig/libdnf.pc
%license COPYING.md
%license lgpl-2.1.txt


# ========== libdnf-cli-devel ==========

%package cli-devel
Summary:        Development files for libdnf-cli
Requires:       libdnf-cli%{?_isa} = %{version}-%{release}

%description cli-devel
Development files for libdnf-cli.

%files cli-devel
%{_includedir}/libdnf-cli/
%{_libdir}/libdnf-cli.so
%{_libdir}/pkgconfig/libdnf-cli.pc
%license COPYING.md
%license lgpl-2.1.txt


# ========== perl5-libdnf ==========

%if %{with perl5}
%package -n perl5-libdnf
Summary:        Perl 5 for the libdnf library.
Provides:       perl(libdnf) = %{version}-%{release}
Requires:       libdnf%{?_isa} = %{version}-%{release}
BuildRequires:  perl-devel
BuildRequires:  swig >= %{swig_version}
%if ! %{with tests_disabled}
BuildRequires:  perl(strict)
BuildRequires:  perl(Test::More)
BuildRequires:  perl(warnings)
%endif

%description -n perl5-libdnf
Perl 5 bindings for the libdnf library.

%files -n perl5-libdnf
%{perl_vendorarch}/libdnf
%{perl_vendorarch}/auto/libdnf
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== perl5-libdnf-cli ==========

%if %{with perl5} && %{with libdnf_cli}
%package -n perl5-libdnf-cli
Summary:        Perl 5 for the libdnf-cli library.
Provides:       perl(libdnf_cli) = %{version}-%{release}
Requires:       libdnf-cli%{?_isa} = %{version}-%{release}
BuildRequires:  perl-devel
BuildRequires:  swig >= %{swig_version}
%if ! %{with tests_disabled}
BuildRequires:  perl(strict)
BuildRequires:  perl(Test::More)
BuildRequires:  perl(warnings)
%endif

%description -n perl5-libdnf-cli
Perl 5 bindings for the libdnf-cli library.

%files -n perl5-libdnf-cli
%{perl_vendorarch}/libdnf_cli
%{perl_vendorarch}/auto/libdnf_cli
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== python3-libdnf ==========

%if %{with python3}
%package -n python3-libdnf
%{?python_provide:%python_provide python3-libdnf}
Summary:        Python 3 bindings for the libdnf library.
Requires:       libdnf%{?_isa} = %{version}-%{release}
BuildRequires:  python3-devel
BuildRequires:  swig >= %{swig_version}

%description -n python3-libdnf
Python 3 bindings for the libdnf library.

%files -n python3-libdnf
%{python3_sitearch}/libdnf/
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== python3-libdnf-cli ==========

%if %{with python3} && %{with libdnf_cli}
%package -n python3-libdnf-cli
%{?python_provide:%python_provide python3-libdnf-cli}
Summary:        Python 3 bindings for the libdnf-cli library.
Requires:       libdnf-cli%{?_isa} = %{version}-%{release}
BuildRequires:  python3-devel
BuildRequires:  swig >= %{swig_version}

%description -n python3-libdnf-cli
Python 3 bindings for the libdnf-cli library.

%files -n python3-libdnf-cli
%{python3_sitearch}/libdnf_cli/
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== ruby-libdnf ==========

%if %{with ruby}
%package -n ruby-libdnf
Summary:        Ruby bindings for the libdnf library.
Provides:       ruby(libdnf) = %{version}-%{release}
Requires:       libdnf%{?_isa} = %{version}-%{release}
Requires:       ruby(release)
BuildRequires:  pkgconfig(ruby)
BuildRequires:  swig >= %{swig_version}

%description -n ruby-libdnf
Ruby bindings for the libdnf library.

%files -n ruby-libdnf
%{ruby_vendorarchdir}/libdnf/
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== ruby-libdnf-cli ==========

%if %{with ruby} && %{with libdnf_cli}
%package -n ruby-libdnf-cli
Summary:        Ruby bindings for the libdnf-cli library.
Provides:       ruby(libdnf_cli) = %{version}-%{release}
Requires:       libdnf-cli%{?_isa} = %{version}-%{release}
BuildRequires:  pkgconfig(ruby)
BuildRequires:  swig >= %{swig_version}

%description -n ruby-libdnf-cli
Ruby bindings for the libdnf-cli library.

%files -n ruby-libdnf-cli
%{ruby_vendorarchdir}/libdnf_cli/
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== dnfdaemon-client ==========

%if %{with dnfdaemon_client}
%package -n dnfdaemon-client
Summary:        Command-line interface for dnfdaemon-server
License:        GPLv2+
Requires:       libdnf%{?_isa} = %{version}-%{release}
Requires:       libdnf-cli%{?_isa} = %{version}-%{release}

%description -n dnfdaemon-client
Command-line interface for dnfdaemon-server

%files -n dnfdaemon-client
%{_bindir}/dnfdaemon-client
%license COPYING.md
%license gpl-2.0.txt
%{_mandir}/man8/dnfdaemon-client.8.gz
%endif


# ========== dnfdaemon-server ==========

%if %{with dnfdaemon_server}
%package -n dnfdaemon-server
Summary:        Package management service with a DBus interface
License:        GPLv2+
Requires:       libdnf%{?_isa} = %{version}-%{release}
Requires:       libdnf-cli%{?_isa} = %{version}-%{release}
Requires:       dnf-data

%description -n dnfdaemon-server
Package management service with a DBus interface

%files -n dnfdaemon-server
%{_bindir}/dnfdaemon-server
%license COPYING.md
%license gpl-2.0.txt
%{_mandir}/man8/dnfdaemon-server.8.gz
%endif


# ========== microdnf ==========

%if %{with microdnf}
%package -n microdnf
Summary:        Package management service with a DBus interface
License:        GPLv2+
Requires:       libdnf%{?_isa} = %{version}-%{release}
Requires:       libdnf-cli%{?_isa} = %{version}-%{release}
Requires:       dnf-data

%description -n microdnf
Package management service with a DBus interface

%files -n microdnf
%{_bindir}/microdnf
%license COPYING.md
%license gpl-2.0.txt
%{_mandir}/man8/microdnf.8.gz
%endif


# ========== unpack, build, check & install ==========

%prep
%autosetup -p1


%build
%cmake \
    -DPACKAGE_VERSION=%{version} \
    -DPERL_INSTALLDIRS=vendor \
    \
    -DWITH_DNFDAEMON_CLIENT=%{?with_dnfdaemon_client:ON}%{!?with_dnfdaemon_client:OFF} \
    -DWITH_DNFDAEMON_SERVER=%{?with_dnfdaemon_server:ON}%{!?with_dnfdaemon_server:OFF} \
    -DWITH_LIBDNF_CLI=%{?with_libdnf_cli:ON}%{!?with_libdnf_cli:OFF} \
    -DWITH_MICRODNF=%{?with_microdnf:ON}%{!?with_microdnf:OFF} \
    \
    -DWITH_COMPS=%{?with_comps:ON}%{!?with_comps:OFF} \
    -DWITH_MODULEMD=%{?with_modulemd:ON}%{!?with_modulemd:OFF} \
    -DWITH_ZCHUNK=%{?with_zchunk:ON}%{!?with_zchunk:OFF} \
    \
    -DWITH_HTML=%{?with_html:ON}%{!?with_html:OFF} \
    -DWITH_MAN=%{?with_man:ON}%{!?with_man:OFF} \
    \
    -DWITH_GO=%{?with_go:ON}%{!?with_go:OFF} \
    -DWITH_PERL5=%{?with_perl5:ON}%{!?with_perl5:OFF} \
    -DWITH_PYTHON3=%{?with_python3:ON}%{!?with_python3:OFF} \
    -DWITH_RUBY=%{?with_ruby:ON}%{!?with_ruby:OFF} \
    \
    -DWITH_SANITIZERS=%{?with_sanitizers:ON}%{!?with_sanitizers:OFF} \
    -DWITH_TESTS=%{?with_tests:ON}%{!?with_tests:OFF} \
    -DWITH_PERFORMANCE_TESTS=%{?with_performance_tests:ON}%{!?with_performance_tests:OFF} \
%make_build
%if %{with man}
    make doc-man
%endif


%check
%if %{with tests}
    make ARGS="-V" test
%endif


%install
%make_install


#find_lang {name}


%ldconfig_scriptlets


%changelog
