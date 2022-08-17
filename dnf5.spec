%global project_version_major 5
%global project_version_minor 0
%global project_version_patch 0

Name:           dnf5
Version:        %{project_version_major}.%{project_version_minor}.%{project_version_patch}
Release:        0~pre%{?dist}
Summary:        Command-line package manager
License:        GPLv2+
URL:            https://github.com/rpm-software-management/dnf5
Source0:        %{url}/archive/%{version}/dnf5-%{version}.tar.gz

Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       dnf-data
Recommends:     bash-completion
BuildRequires:  bash-completion

# ========== build options ==========

%bcond_without dnf5daemon_client
%bcond_without dnf5daemon_server
%bcond_without libdnf_cli
%bcond_without dnf5
%bcond_without dnf5_plugins
%bcond_without plugin_actions
%bcond_without python_plugins_loader

%bcond_without comps
%bcond_without modulemd
%bcond_without zchunk

%bcond_with    html
%if 0%{?rhel} == 8
%bcond_with    man
%else
%bcond_without man
%endif

# TODO Go bindings fail to build, disable for now
%bcond_with    go
%bcond_without perl5
%bcond_without python3
%bcond_without ruby

%bcond_with    clang
%bcond_with    sanitizers
%bcond_without tests
%bcond_with    performance_tests
%bcond_with    dnf5daemon_tests

%if %{with clang}
    %global toolchain clang
%endif

# ========== versions of dependencies ==========

%global libmodulemd_version 2.5.0
%global librepo_version 1.13.0
%global libsolv_version 0.7.21
%global swig_version 4
%global zchunk_version 0.9.11


# ========== build requires ==========

BuildRequires:  cmake
BuildRequires:  doxygen
BuildRequires:  gettext
%if %{with clang}
BuildRequires:  clang
%else
BuildRequires:  gcc-c++
%endif

BuildRequires:  toml11-devel
BuildRequires:  pkgconfig(check)
%if %{with tests}
BuildRequires:  pkgconfig(cppunit)
%endif
BuildRequires:  pkgconfig(fmt)
BuildRequires:  (pkgconfig(gpgme) or gpgme-devel)
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
BuildRequires:  pkgconfig(rpm) >= 4.17.0
BuildRequires:  pkgconfig(sqlite3)
%if %{with zchunk}
BuildRequires:  pkgconfig(zck) >= %{zchunk_version}
%endif

%if %{with html} || %{with man}
BuildRequires:  python3dist(breathe)
BuildRequires:  python3dist(sphinx) >= 4.1.2
BuildRequires:  python3dist(sphinx-rtd-theme)
%endif

%if %{with tests}
BuildRequires:  rpm-build
BuildRequires:  createrepo_c
%endif

%if %{with sanitizers}
# compiler-rt is required by sanitizers in clang
BuildRequires:  compiler-rt
BuildRequires:  libasan
BuildRequires:  liblsan
BuildRequires:  libubsan
%endif


%description
DNF5 is a command-line package manager that automates the process of installing,
upgrading, configuring, and removing computer programs in a consistent manner.
It supports RPM packages, modulemd modules, and comps groups & environments.

%files
%{_bindir}/dnf5
%dir %{_libdir}/dnf5/
%dir %{_libdir}/dnf5/plugins/
%{_libdir}/dnf5/plugins/README
%dir %{_datadir}/bash-completion/
%dir %{_datadir}/bash-completion/completions/
%{_datadir}/bash-completion/completions/dnf5
%license COPYING.md
%license gpl-2.0.txt
%{_mandir}/man8/dnf5.8.gz

# ========== libdnf5 ==========
%package -n libdnf5
Summary:        Package management library
License:        LGPLv2.1+
#Requires:       libmodulemd{?_isa} >= {libmodulemd_version}
Requires:       libsolv%{?_isa} >= %{libsolv_version}
Requires:       librepo%{?_isa} >= %{librepo_version}

%description -n libdnf5
Package management library

%files -n libdnf5
%{_libdir}/libdnf.so.*
%license lgpl-2.1.txt
%{_var}/cache/libdnf/

# ========== libdnf5-cli ==========

%if %{with libdnf_cli}
%package -n libdnf5-cli
Summary:        Library for working with a terminal in a command-line package manager
License:        LGPLv2.1+
BuildRequires:  pkgconfig(smartcols)
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description -n libdnf5-cli
Library for working with a terminal in a command-line package manager.

%files -n libdnf5-cli
%{_libdir}/libdnf-cli.so.*
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== libdnf5-devel ==========

%package -n libdnf5-devel
Summary:        Development files for libdnf
License:        LGPLv2.1+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       libsolv-devel%{?_isa} >= %{libsolv_version}
Conflicts:      libdnf-devel < 5

%description -n libdnf5-devel
Development files for libdnf.

%files -n libdnf5-devel
%{_includedir}/libdnf/
%{_libdir}/libdnf.so
%{_libdir}/pkgconfig/libdnf.pc
%license COPYING.md
%license lgpl-2.1.txt


# ========== libdnf5-cli-devel ==========

%package -n libdnf5-cli-devel
Summary:        Development files for libdnf5-cli
License:        LGPLv2.1+
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}

%description -n libdnf5-cli-devel
Development files for libdnf5-cli.

%files -n libdnf5-cli-devel
%{_includedir}/libdnf-cli/
%{_libdir}/libdnf-cli.so
%{_libdir}/pkgconfig/libdnf-cli.pc
%license COPYING.md
%license lgpl-2.1.txt


# ========== perl5-libdnf ==========

%if %{with perl5}
%package -n perl5-libdnf5
Summary:        Perl 5 bindings for the libdnf library.
License:        LGPLv2.1+
Provides:       perl(libdnf) = %{version}-%{release}
Requires:       %{name}%{?_isa} = %{version}-%{release}
BuildRequires:  perl-devel
BuildRequires:  perl-macros
BuildRequires:  swig >= %{swig_version}
%if %{with tests}
BuildRequires:  perl(strict)
BuildRequires:  perl(Test::More)
BuildRequires:  perl(Test::Exception)
BuildRequires:  perl(warnings)
%endif

%description -n perl5-libdnf5
Perl 5 bindings for the libdnf library.

%files -n perl5-libdnf5
%{perl_vendorarch}/libdnf5
%{perl_vendorarch}/auto/libdnf5
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== perl5-libdnf5-cli ==========

%if %{with perl5} && %{with libdnf_cli}
%package -n perl5-libdnf5-cli
Summary:        Perl 5 bindings for the libdnf5-cli library.
License:        LGPLv2.1+
Provides:       perl(libdnf_cli) = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
BuildRequires:  perl-devel
BuildRequires:  perl-macros
BuildRequires:  swig >= %{swig_version}
%if %{with tests}
BuildRequires:  perl(strict)
BuildRequires:  perl(Test::More)
BuildRequires:  perl(Test::Exception)
BuildRequires:  perl(warnings)
%endif

%description -n perl5-libdnf5-cli
Perl 5 bindings for the libdnf5-cli library.

%files -n perl5-libdnf5-cli
%{perl_vendorarch}/libdnf5_cli
%{perl_vendorarch}/auto/libdnf5_cli
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== python3-libdnf5 ==========

%if %{with python3}
%package -n python3-libdnf5
%{?python_provide:%python_provide python3-libdnf}
Summary:        Python 3 bindings for the libdnf library.
License:        LGPLv2.1+
Requires:       %{name}%{?_isa} = %{version}-%{release}
BuildRequires:  python3-devel
BuildRequires:  swig >= %{swig_version}

%description -n python3-libdnf5
Python 3 bindings for the libdnf library.

%files -n python3-libdnf5
%{python3_sitearch}/libdnf5/
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== python3-libdnf5-cli ==========

%if %{with python3} && %{with libdnf_cli}
%package -n python3-libdnf5-cli
%{?python_provide:%python_provide python3-libdnf5-cli}
Summary:        Python 3 bindings for the libdnf5-cli library.
License:        LGPLv2.1+
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
BuildRequires:  python3-devel
BuildRequires:  swig >= %{swig_version}

%description -n python3-libdnf5-cli
Python 3 bindings for the libdnf5-cli library.

%files -n python3-libdnf5-cli
%{python3_sitearch}/libdnf5_cli/
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== ruby-libdnf5 ==========

%if %{with ruby}
%package -n ruby-libdnf5
Summary:        Ruby bindings for the libdnf library.
License:        LGPLv2.1+
Provides:       ruby(libdnf) = %{version}-%{release}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       ruby(release)
BuildRequires:  pkgconfig(ruby)
BuildRequires:  swig >= %{swig_version}
%if %{with tests}
BuildRequires:  rubygem-test-unit
%endif

%description -n ruby-libdnf5
Ruby bindings for the libdnf library.

%files -n ruby-libdnf5
%{ruby_vendorarchdir}/libdnf5/
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== ruby-libdnf5-cli ==========

%if %{with ruby} && %{with libdnf_cli}
%package -n ruby-libdnf5-cli
Summary:        Ruby bindings for the libdnf5-cli library.
License:        LGPLv2.1+
Provides:       ruby(libdnf_cli) = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
BuildRequires:  pkgconfig(ruby)
BuildRequires:  swig >= %{swig_version}
%if %{with tests}
BuildRequires:  rubygem-test-unit
%endif

%description -n ruby-libdnf5-cli
Ruby bindings for the libdnf5-cli library.

%files -n ruby-libdnf5-cli
%{ruby_vendorarchdir}/libdnf5_cli/
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== libdnf5-plugin-actions ==========

%if %{with plugin_actions}
%package -n libdnf5-plugin-actions
Summary:        Libdnf plugin that allows to run actions (external executables) on hooks
License:        LGPLv2.1+
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description -n libdnf5-plugin-actions
Libdnf plugin that allows to run actions (external executables) on hooks.

%files -n libdnf5-plugin-actions
%{_libdir}/libdnf5/plugins/actions.*
%endif


# ========== python3-libdnf5-plugins-loader ==========

%if %{with python_plugins_loader}
%package -n python3-libdnf5-python-plugins-loader
Summary:        Libdnf plugin that allows loading Python plugins
License:        LGPLv2.1+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       python3-libdnf5%{?_isa} = %{version}-%{release}

%description -n python3-libdnf5-python-plugins-loader
Libdnf plugin that allows loading Python plugins

%files -n python3-libdnf5-python-plugins-loader
%{_libdir}/libdnf5/plugins/python_plugins_loader.*
%{python3_sitelib}/libdnf_plugins/
%{python3_sitelib}/libdnf_plugins/README
%endif


# ========== dnf5daemon-client ==========

%if %{with dnf5daemon_client}
%package -n dnf5daemon-client
Summary:        Command-line interface for dnf5daemon-server
License:        GPLv2+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
Requires:       dnf5daemon-server

%description -n dnf5daemon-client
Command-line interface for dnf5daemon-server

%files -n dnf5daemon-client
%{_bindir}/dnf5daemon-client
%license COPYING.md
%license gpl-2.0.txt
%{_mandir}/man8/dnf5daemon-client.8.gz
%endif


# ========== dnf5daemon-server ==========

%if %{with dnf5daemon_server}
%package -n dnf5daemon-server
Summary:        Package management service with a DBus interface
License:        GPLv2+
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
Requires:       dnf-data
%{?systemd_requires}
BuildRequires:  pkgconfig(sdbus-c++) >= 0.8.1
BuildRequires:  systemd-rpm-macros
%if %{with dnf5daemon_tests}
BuildRequires:  dbus-daemon
BuildRequires:  polkit
BuildRequires:  python3-devel
BuildRequires:  python3dist(dbus-python)
%endif

%description -n dnf5daemon-server
Package management service with a DBus interface

%post -n dnf5daemon-server
%systemd_post dnf5daemon-server.service

%preun -n dnf5daemon-server
%systemd_preun dnf5daemon-server.service

%postun -n dnf5daemon-server
%systemd_postun_with_restart dnf5daemon-server.service

%files -n dnf5daemon-server
%{_bindir}/dnf5daemon-server
%{_unitdir}/dnf5daemon-server.service
%{_sysconfdir}/dbus-1/system.d/org.rpm.dnf.v0.conf
%{_datadir}/dbus-1/system-services/org.rpm.dnf.v0.service
%{_datadir}/dbus-1/interfaces/org.rpm.dnf.v0.*.xml
%{_datadir}/polkit-1/actions/org.rpm.dnf.v0.policy
%license COPYING.md
%license gpl-2.0.txt
%{_mandir}/man8/dnf5daemon-server.8.gz
%{_mandir}/man8/dnf5daemon-dbus-api.8.gz
%endif


# ========== dnf5-plugins ==========

%if %{with dnf5_plugins}
%package -n dnf5-plugins
Summary:        dnf5 plugins
License:        LGPLv2.1+
Requires:       dnf5%{?_isa} = %{version}-%{release}

%description -n dnf5-plugins
DNF5 plugins

%files -n dnf5-plugins
%{_libdir}/dnf5/plugins/*.so
%endif


# ========== unpack, build, check & install ==========

%prep
%autosetup -p1 -n dnf5-%{version}


%build
%cmake \
    -DPACKAGE_VERSION=%{version} \
    -DPERL_INSTALLDIRS=vendor \
    \
    -DWITH_DNF5DAEMON_CLIENT=%{?with_dnf5daemon_client:ON}%{!?with_dnf5daemon_client:OFF} \
    -DWITH_DNF5DAEMON_SERVER=%{?with_dnf5daemon_server:ON}%{!?with_dnf5daemon_server:OFF} \
    -DWITH_LIBDNF_CLI=%{?with_libdnf_cli:ON}%{!?with_libdnf_cli:OFF} \
    -DWITH_DNF5=%{?with_dnf5:ON}%{!?with_dnf5:OFF} \
    -DWITH_PLUGIN_ACTIONS=%{?with_plugin_actions:ON}%{!?with_plugin_actions:OFF} \
    -DWITH_PYTHON_PLUGINS_LOADER=%{?with_python_plugins_loader:ON}%{!?with_python_plugins_loader:OFF} \
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
    -DWITH_DNF5DAEMON_TESTS=%{?with_dnf5daemon_tests:ON}%{!?with_dnf5daemon_tests:OFF} \
    \
    -DPROJECT_VERSION_MAJOR=%{project_version_major} \
    -DPROJECT_VERSION_MINOR=%{project_version_minor} \
    -DPROJECT_VERSION_PATCH=%{project_version_patch}
%cmake_build
%if %{with man}
    %cmake_build --target doc-man
%endif


%check
%if %{with tests}
    %ctest
%endif


%install
%cmake_install

#find_lang {name}


%ldconfig_scriptlets


%changelog
