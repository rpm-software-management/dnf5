%global project_version_major 5
%global project_version_minor 1
%global project_version_patch 16

%bcond dnf5_obsoletes_dnf %[0%{?fedora} > 41 || 0%{?rhel} > 11]

Name:           dnf5
Version:        %{project_version_major}.%{project_version_minor}.%{project_version_patch}
Release:        1%{?dist}
Summary:        Command-line package manager
License:        GPL-2.0-or-later
URL:            https://github.com/rpm-software-management/dnf5
Source0:        %{url}/archive/%{version}/dnf5-%{version}.tar.gz

Requires:       libdnf5%{?_isa} = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
%if %{without dnf5_obsoletes_dnf}
Requires:       dnf-data
%endif
Recommends:     bash-completion
Requires:       coreutils

# Remove if condition when Fedora 37 is EOL
%if 0%{?fedora} > 37 || 0%{?rhel} > 10
Provides:       microdnf = %{version}-%{release}
Obsoletes:      microdnf < 4
%endif

%if %{with dnf5_obsoletes_dnf}
Provides:       dnf = %{version}-%{release}
Obsoletes:      dnf < 5

Provides:       yum = %{version}-%{release}
Obsoletes:      yum < 5
%endif

Provides:       dnf5-command(install)
Provides:       dnf5-command(upgrade)
Provides:       dnf5-command(remove)
Provides:       dnf5-command(distro-sync)
Provides:       dnf5-command(downgrade)
Provides:       dnf5-command(reinstall)
Provides:       dnf5-command(swap)
Provides:       dnf5-command(mark)
Provides:       dnf5-command(autoremove)
Provides:       dnf5-command(check)
Provides:       dnf5-command(check-upgrade)
Provides:       dnf5-command(provides)

Provides:       dnf5-command(leaves)
Provides:       dnf5-command(repoquery)
Provides:       dnf5-command(search)
Provides:       dnf5-command(list)
Provides:       dnf5-command(info)

Provides:       dnf5-command(group)
Provides:       dnf5-command(environment)
Provides:       dnf5-command(module)
Provides:       dnf5-command(history)
Provides:       dnf5-command(repo)
Provides:       dnf5-command(advisory)

Provides:       dnf5-command(clean)
Provides:       dnf5-command(download)
Provides:       dnf5-command(makecache)
Provides:       dnf5-command(offline)
Provides:       dnf5-command(system-upgrade)


# ========== build options ==========

%bcond_without dnf5daemon_client
%bcond_without dnf5daemon_server
%bcond_without libdnf_cli
%bcond_without dnf5
%bcond_without dnf5_plugins
%bcond_without plugin_actions
%bcond_without plugin_rhsm
%bcond_without python_plugins_loader

%bcond_without comps
%bcond_without modulemd
%if 0%{?rhel}
%bcond_with    zchunk
%else
%bcond_without zchunk
%endif
%bcond_without systemd

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
%global librepo_version 1.15.0
%global libsolv_version 0.7.25
%global sqlite_version 3.35.0
%global swig_version 4
%global zchunk_version 0.9.11


# ========== build requires ==========

%if 0%{?fedora} > 40 || 0%{?rhel} > 10
BuildRequires:  bash-completion-devel
%else
BuildRequires:  bash-completion
%endif
BuildRequires:  cmake
BuildRequires:  doxygen
BuildRequires:  gettext
BuildRequires:  pkgconfig(check)
BuildRequires:  pkgconfig(fmt)
BuildRequires:  pkgconfig(json-c)
BuildRequires:  pkgconfig(libcrypto)
BuildRequires:  pkgconfig(librepo) >= %{librepo_version}
BuildRequires:  pkgconfig(libsolv) >= %{libsolv_version}
BuildRequires:  pkgconfig(libsolvext) >= %{libsolv_version}
BuildRequires:  pkgconfig(rpm) >= 4.17.0
BuildRequires:  pkgconfig(sqlite3) >= %{sqlite_version}
BuildRequires:  toml11-static

%if %{with clang}
BuildRequires:  clang
%else
BuildRequires:  gcc-c++ >= 10.1
%endif

%if %{with tests}
BuildRequires:  createrepo_c
BuildRequires:  pkgconfig(cppunit)
BuildRequires:  rpm-build
%endif

%if %{with comps}
BuildRequires:  pkgconfig(libcomps)
%endif

%if %{with modulemd}
BuildRequires:  pkgconfig(modulemd-2.0) >= %{libmodulemd_version}
%endif

%if %{with zchunk}
BuildRequires:  pkgconfig(zck) >= %{zchunk_version}
%endif

%if %{with systemd}
BuildRequires:  pkgconfig(sdbus-c++) >= 0.8.1
BuildRequires:  systemd-devel

 # We need to get the SYSTEMD_SYSTEM_UNIT_DIR from
 # /usr/share/pkgconfig/systemd.pc
BuildRequires:  systemd
%endif

%if %{with html} || %{with man}
BuildRequires:  python3dist(breathe)
BuildRequires:  python3dist(sphinx) >= 4.1.2
BuildRequires:  python3dist(sphinx-rtd-theme)
%endif

%if %{with sanitizers}
# compiler-rt is required by sanitizers in clang
BuildRequires:  compiler-rt
BuildRequires:  libasan
BuildRequires:  liblsan
BuildRequires:  libubsan
%endif

%if %{with libdnf_cli}
# required for libdnf5-cli
BuildRequires:  pkgconfig(smartcols)
%endif

%if %{with dnf5_plugins}
BuildRequires:  libcurl-devel >= 7.62.0
%endif

%if %{with dnf5daemon_server}
# required for dnf5daemon-server
BuildRequires:  pkgconfig(sdbus-c++) >= 0.9.0
BuildRequires:  systemd-rpm-macros
%if %{with dnf5daemon_tests}
BuildRequires:  dbus-daemon
BuildRequires:  polkit
BuildRequires:  python3-devel
BuildRequires:  python3dist(dbus-python)
%endif
%endif

%if %{with plugin_rhsm}
BuildRequires:  pkgconfig(librhsm) >= 0.0.3
BuildRequires:  pkgconfig(glib-2.0) >= 2.44.0
%endif

# ========== language bindings section ==========

%if %{with perl5} || %{with ruby} || %{with python3}
BuildRequires:  swig >= %{swig_version}
%endif

%if %{with perl5}
# required for perl-libdnf5 and perl-libdnf5-cli
BuildRequires:  perl-devel
BuildRequires:  perl-generators
%if %{with tests}
BuildRequires:  perl(strict)
BuildRequires:  perl(Test::More)
BuildRequires:  perl(Test::Exception)
BuildRequires:  perl(warnings)
%endif
%endif

%if %{with ruby}
# required for ruby-libdnf5 and ruby-libdnf5-cli
BuildRequires:  pkgconfig(ruby)
%if %{with tests}
BuildRequires:  rubygem-test-unit
%endif
%endif

%if %{with python3}
# required for python3-libdnf5 and python3-libdnf5-cli
BuildRequires:  python3-devel
%endif

%description
DNF5 is a command-line package manager that automates the process of installing,
upgrading, configuring, and removing computer programs in a consistent manner.
It supports RPM packages, modulemd modules, and comps groups & environments.

%files -f dnf5.lang
%{_bindir}/dnf5
%if %{with dnf5_obsoletes_dnf}
%{_bindir}/dnf
%{_bindir}/yum
%endif

# Remove if condition when Fedora 37 is EOL
%if 0%{?fedora} > 37 || 0%{?rhel} > 10
%{_bindir}/microdnf
%endif

%dir %{_sysconfdir}/dnf/dnf5-aliases.d
%doc %{_sysconfdir}/dnf/dnf5-aliases.d/README
%dir %{_datadir}/dnf5
%dir %{_datadir}/dnf5/aliases.d
%config %{_datadir}/dnf5/aliases.d/compatibility.conf
%dir %{_libdir}/dnf5
%dir %{_libdir}/dnf5/plugins
%dir %{_datadir}/dnf5/dnf5-plugins
%dir %{_sysconfdir}/dnf/dnf5-plugins
%doc %{_libdir}/dnf5/plugins/README
%dir %{_libdir}/libdnf5/plugins
%dir %{_datadir}/bash-completion/
%dir %{_datadir}/bash-completion/completions/
%{_datadir}/bash-completion/completions/dnf5
%if %{with dnf5_obsoletes_dnf}
%{_datadir}/bash-completion/completions/dnf
%endif
%dir %{_prefix}/lib/sysimage/dnf
%verify(not md5 size mtime) %ghost %{_prefix}/lib/sysimage/dnf/*
%license COPYING.md
%license gpl-2.0.txt
%{_mandir}/man8/dnf5.8.*
%{_mandir}/man8/dnf5-advisory.8.*
%{_mandir}/man8/dnf5-autoremove.8.*
%{_mandir}/man8/dnf5-check.8.*
%{_mandir}/man8/dnf5-check-upgrade.8.*
%{_mandir}/man8/dnf5-clean.8.*
%{_mandir}/man8/dnf5-distro-sync.8.*
%{_mandir}/man8/dnf5-downgrade.8.*
%{_mandir}/man8/dnf5-download.8.*
%{_mandir}/man8/dnf5-environment.8.*
%{_mandir}/man8/dnf5-group.8.*
# TODO(jkolarik): history is not ready yet
# %%{_mandir}/man8/dnf5-history.8.*
%{_mandir}/man8/dnf5-info.8.*
%{_mandir}/man8/dnf5-install.8.*
%{_mandir}/man8/dnf5-leaves.8.*
%{_mandir}/man8/dnf5-list.8.*
%{_mandir}/man8/dnf5-makecache.8.*
%{_mandir}/man8/dnf5-mark.8.*
%{_mandir}/man8/dnf5-module.8.*
%{_mandir}/man8/dnf5-offline.8.*
%{_mandir}/man8/dnf5-provides.8.*
%{_mandir}/man8/dnf5-reinstall.8.*
%{_mandir}/man8/dnf5-remove.8.*
%{_mandir}/man8/dnf5-repo.8.*
%{_mandir}/man8/dnf5-repoquery.8.*
%{_mandir}/man8/dnf5-search.8.*
%{_mandir}/man8/dnf5-swap.8.*
%{_mandir}/man8/dnf5-system-upgrade.8.*
%{_mandir}/man8/dnf5-upgrade.8.*
%{_mandir}/man8/dnf5-versionlock.8.*
%{_mandir}/man7/dnf5-aliases.7.*
%{_mandir}/man7/dnf5-caching.7.*
%{_mandir}/man7/dnf5-comps.7.*
# TODO(jkolarik): filtering is not ready yet
# %%{_mandir}/man7/dnf5-filtering.7.*
%{_mandir}/man7/dnf5-forcearch.7.*
%{_mandir}/man7/dnf5-installroot.7.*
# TODO(jkolarik): modularity is not ready yet
# %%{_mandir}/man7/dnf5-modularity.7.*
%{_mandir}/man7/dnf5-specs.7.*
%{_mandir}/man5/dnf5.conf.5.*
%{_mandir}/man5/dnf5.conf-todo.5.*
%{_mandir}/man5/dnf5.conf-deprecated.5.*

%if %{with systemd}
%{_unitdir}/dnf5-offline-transaction.service
%{_unitdir}/dnf5-offline-transaction-cleanup.service
%{_unitdir}/system-update.target.wants/dnf5-offline-transaction.service
%endif

# ========== libdnf5 ==========
%package -n libdnf5
Summary:        Package management library
License:        LGPL-2.1-or-later
#Requires:       libmodulemd{?_isa} >= {libmodulemd_version}
Requires:       libsolv%{?_isa} >= %{libsolv_version}
Requires:       librepo%{?_isa} >= %{librepo_version}
Requires:       sqlite-libs%{?_isa} >= %{sqlite_version}
%if %{with dnf5_obsoletes_dnf}
Conflicts:      dnf-data < 4.16.0
%endif

%description -n libdnf5
Package management library.

%files -n libdnf5 -f libdnf5.lang
%if %{with dnf5_obsoletes_dnf}
%config(noreplace) %{_sysconfdir}/dnf/dnf.conf
%dir %{_sysconfdir}/dnf/vars
%dir %{_sysconfdir}/dnf/protected.d
%else
%exclude %{_sysconfdir}/dnf/dnf.conf
%endif
%dir %{_datadir}/dnf5/libdnf.conf.d
%dir %{_sysconfdir}/dnf/libdnf5.conf.d
%dir %{_datadir}/dnf5/repos.override.d
%dir %{_sysconfdir}/dnf/repos.override.d
%dir %{_sysconfdir}/dnf/libdnf5-plugins
%dir %{_datadir}/dnf5/repos.d
%dir %{_datadir}/dnf5/vars.d
%dir %{_libdir}/libdnf5
%{_libdir}/libdnf5.so.1*
%license lgpl-2.1.txt
%{_var}/cache/libdnf5/

# ========== libdnf5-cli ==========

%if %{with libdnf_cli}
%package -n libdnf5-cli
Summary:        Library for working with a terminal in a command-line package manager
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}

%description -n libdnf5-cli
Library for working with a terminal in a command-line package manager.

%files -n libdnf5-cli -f libdnf5-cli.lang
%{_libdir}/libdnf5-cli.so.1*
%license COPYING.md
%license lgpl-2.1.txt
%endif

# ========== dnf5-devel ==========

%package -n dnf5-devel
Summary:        Development files for dnf5
License:        LGPL-2.1-or-later
Requires:       dnf5%{?_isa} = %{version}-%{release}
Requires:       libdnf5-devel%{?_isa} = %{version}-%{release}
Requires:       libdnf5-cli-devel%{?_isa} = %{version}-%{release}

%description -n dnf5-devel
Development files for dnf5.

%files -n dnf5-devel
%{_includedir}/dnf5/
%license COPYING.md
%license lgpl-2.1.txt


# ========== libdnf5-devel ==========

%package -n libdnf5-devel
Summary:        Development files for libdnf
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}
Requires:       libsolv-devel%{?_isa} >= %{libsolv_version}

%description -n libdnf5-devel
Development files for libdnf.

%files -n libdnf5-devel
%{_includedir}/libdnf5/
%dir %{_libdir}/libdnf5
%{_libdir}/libdnf5.so
%{_libdir}/pkgconfig/libdnf5.pc
%license COPYING.md
%license lgpl-2.1.txt


# ========== libdnf5-cli-devel ==========

%package -n libdnf5-cli-devel
Summary:        Development files for libdnf5-cli
License:        LGPL-2.1-or-later
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}

%description -n libdnf5-cli-devel
Development files for libdnf5-cli.

%files -n libdnf5-cli-devel
%{_includedir}/libdnf5-cli/
%{_libdir}/libdnf5-cli.so
%{_libdir}/pkgconfig/libdnf5-cli.pc
%license COPYING.md
%license lgpl-2.1.txt


# ========== perl-libdnf5 ==========

%if %{with perl5}
%package -n perl-libdnf5
Summary:        Perl 5 bindings for the libdnf library
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}


%description -n perl-libdnf5
Perl 5 bindings for the libdnf library.

%files -n perl-libdnf5
%{perl_vendorarch}/libdnf5
%{perl_vendorarch}/auto/libdnf5
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== perl-libdnf5-cli ==========

%if %{with perl5} && %{with libdnf_cli}
%package -n perl-libdnf5-cli
Summary:        Perl 5 bindings for the libdnf5-cli library
License:        LGPL-2.1-or-later
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}


%description -n perl-libdnf5-cli
Perl 5 bindings for the libdnf5-cli library.

%files -n perl-libdnf5-cli
%{perl_vendorarch}/libdnf5_cli
%{perl_vendorarch}/auto/libdnf5_cli
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== python3-libdnf5 ==========

%if %{with python3}
%package -n python3-libdnf5
%{?python_provide:%python_provide python3-libdnf}
Summary:        Python 3 bindings for the libdnf library
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}

%description -n python3-libdnf5
Python 3 bindings for the libdnf library.

%files -n python3-libdnf5
%{python3_sitearch}/libdnf5
%{python3_sitearch}/libdnf5-*.dist-info
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== python3-libdnf5-cli ==========

%if %{with python3} && %{with libdnf_cli}
%package -n python3-libdnf5-cli
%{?python_provide:%python_provide python3-libdnf5-cli}
Summary:        Python 3 bindings for the libdnf5-cli library
License:        LGPL-2.1-or-later
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}

%description -n python3-libdnf5-cli
Python 3 bindings for the libdnf5-cli library.

%files -n python3-libdnf5-cli
%{python3_sitearch}/libdnf5_cli
%{python3_sitearch}/libdnf5_cli-*.dist-info
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== ruby-libdnf5 ==========

%if %{with ruby}
%package -n ruby-libdnf5
Summary:        Ruby bindings for the libdnf library
License:        LGPL-2.1-or-later
Provides:       ruby(libdnf) = %{version}-%{release}
Requires:       libdnf5%{?_isa} = %{version}-%{release}
Requires:       ruby(release)

%description -n ruby-libdnf5
Ruby bindings for the libdnf library.

%files -n ruby-libdnf5
%{ruby_vendorarchdir}/libdnf5
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== ruby-libdnf5-cli ==========

%if %{with ruby} && %{with libdnf_cli}
%package -n ruby-libdnf5-cli
Summary:        Ruby bindings for the libdnf5-cli library
License:        LGPL-2.1-or-later
Provides:       ruby(libdnf_cli) = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
Requires:       ruby(release)

%description -n ruby-libdnf5-cli
Ruby bindings for the libdnf5-cli library.

%files -n ruby-libdnf5-cli
%{ruby_vendorarchdir}/libdnf5_cli
%license COPYING.md
%license lgpl-2.1.txt
%endif


# ========== libdnf5-plugin-actions ==========

%if %{with plugin_actions}
%package -n libdnf5-plugin-actions
Summary:        Libdnf5 plugin that allows to run actions (external executables) on hooks
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}

%description -n libdnf5-plugin-actions
Libdnf5 plugin that allows to run actions (external executables) on hooks.

%files -n libdnf5-plugin-actions -f libdnf5-plugin-actions.lang
%{_libdir}/libdnf5/plugins/actions.*
%config %{_sysconfdir}/dnf/libdnf5-plugins/actions.conf
%dir %{_sysconfdir}/dnf/libdnf5-plugins/actions.d
%{_mandir}/man8/libdnf5-actions.8.*
%endif


# ========== libdnf5-plugin-plugin_rhsm ==========

%if %{with plugin_rhsm}
%package -n libdnf5-plugin-rhsm
Summary:        Libdnf5 rhsm (Red Hat Subscription Manager) plugin
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}

%description -n libdnf5-plugin-rhsm
Libdnf5 plugin with basic support for Red Hat subscriptions.
Synchronizes the the enrollment with the vendor system. This can change
the contents of the repositories configuration files according
to the subscription levels.

%files -n libdnf5-plugin-rhsm -f libdnf5-plugin-rhsm.lang
%{_libdir}/libdnf5/plugins/rhsm.*
%config %{_sysconfdir}/dnf/libdnf5-plugins/rhsm.conf
%endif


# ========== python3-libdnf5-plugins-loader ==========

%if %{with python_plugins_loader}
%package -n python3-libdnf5-python-plugins-loader
Summary:        Libdnf5 plugin that allows loading Python plugins
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}
Requires:       python3-libdnf5%{?_isa} = %{version}-%{release}

%description -n python3-libdnf5-python-plugins-loader
Libdnf5 plugin that allows loading Python plugins.

%files -n python3-libdnf5-python-plugins-loader
%{_libdir}/libdnf5/plugins/python_plugins_loader.*
%dir %{python3_sitelib}/libdnf_plugins/
%doc %{python3_sitelib}/libdnf_plugins/README
%endif


# ========== dnf5daemon-client ==========

%if %{with dnf5daemon_client}
%package -n dnf5daemon-client
Summary:        Command-line interface for dnf5daemon-server
License:        GPL-2.0-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
Requires:       dnf5daemon-server

%description -n dnf5daemon-client
Command-line interface for dnf5daemon-server.

%files -n dnf5daemon-client -f dnf5daemon-client.lang
%{_bindir}/dnf5daemon-client
%license COPYING.md
%license gpl-2.0.txt
%{_mandir}/man8/dnf5daemon-client.8.*
%endif


# ========== dnf5daemon-server ==========

%if %{with dnf5daemon_server}
%package -n dnf5daemon-server
Summary:        Package management service with a DBus interface
License:        GPL-2.0-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
Requires:       dbus
Requires:       polkit
%if %{without dnf5_obsoletes_dnf}
Requires:       dnf-data
%endif

%description -n dnf5daemon-server
Package management service with a DBus interface.

%post -n dnf5daemon-server
%systemd_post dnf5daemon-server.service

%preun -n dnf5daemon-server
%systemd_preun dnf5daemon-server.service

%postun -n dnf5daemon-server
%systemd_postun_with_restart dnf5daemon-server.service

%files -n dnf5daemon-server -f dnf5daemon-server.lang
%{_sbindir}/dnf5daemon-server
%{_unitdir}/dnf5daemon-server.service
%config(noreplace) %{_sysconfdir}/dbus-1/system.d/org.rpm.dnf.v0.conf
%{_datadir}/dbus-1/system-services/org.rpm.dnf.v0.service
%{_datadir}/dbus-1/interfaces/org.rpm.dnf.v0.*.xml
%{_datadir}/polkit-1/actions/org.rpm.dnf.v0.policy
%license COPYING.md
%license gpl-2.0.txt
%{_mandir}/man8/dnf5daemon-server.8.*
%{_mandir}/man8/dnf5daemon-dbus-api.8.*
%endif


# ========== dnf5-plugins ==========
%if %{with dnf5_plugins}

%package -n dnf5-plugins
Summary:        Plugins for dnf5
License:        LGPL-2.1-or-later
Requires:       dnf5%{?_isa} = %{version}-%{release}
Requires:       libcurl%{?_isa} >= 7.62.0
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
Provides:       dnf5-command(builddep)
Provides:       dnf5-command(changelog)
Provides:       dnf5-command(config-manager)
Provides:       dnf5-command(copr)
Provides:       dnf5-command(needs-restarting)
Provides:       dnf5-command(repoclosure)

%description -n dnf5-plugins
Core DNF5 plugins that enhance dnf5 with builddep, changelog,
config-manager, copr, and repoclosure commands.

%files -n dnf5-plugins -f dnf5-plugin-builddep.lang -f dnf5-plugin-changelog.lang -f dnf5-plugin-config-manager.lang -f dnf5-plugin-copr.lang -f dnf5-plugin-needs-restarting.lang -f dnf5-plugin-repoclosure.lang
%{_libdir}/dnf5/plugins/builddep_cmd_plugin.so
%{_libdir}/dnf5/plugins/changelog_cmd_plugin.so
%{_libdir}/dnf5/plugins/config-manager_cmd_plugin.so
%{_libdir}/dnf5/plugins/copr_cmd_plugin.so
%{_libdir}/dnf5/plugins/needs_restarting_cmd_plugin.so
%{_libdir}/dnf5/plugins/repoclosure_cmd_plugin.so
%{_mandir}/man8/dnf5-builddep.8.*
%{_mandir}/man8/dnf5-changelog.8.*
%{_mandir}/man8/dnf5-copr.8.*
%{_mandir}/man8/dnf5-needs-restarting.8.*
%{_mandir}/man8/dnf5-repoclosure.8.*


# ========== dnf5-automatic plugin ==========

%package plugin-automatic
Summary:        Package manager - automated upgrades
License:        LGPL-2.1-or-later
Requires:       dnf5%{?_isa} = %{version}-%{release}
Requires:       libcurl-full%{?_isa}
Provides:       dnf5-command(automatic)
%if %{with dnf5_obsoletes_dnf}
Provides:       dnf-automatic = %{version}-%{release}
Obsoletes:      dnf-automatic < 5
%else
Conflicts:      dnf-automatic < 5
%endif

%description plugin-automatic
Alternative command-line interface "dnf upgrade" suitable to be executed
automatically and regularly from systemd timers, cron jobs or similar.

%files plugin-automatic -f dnf5-plugin-automatic.lang
%ghost %{_sysconfdir}/motd.d/dnf5-automatic
%{_libdir}/dnf5/plugins/automatic_cmd_plugin.so
%{_mandir}/man8/dnf5-automatic.8.*
%{_unitdir}/dnf5-automatic.service
%{_unitdir}/dnf5-automatic.timer
%{_unitdir}/dnf-automatic.service
%{_unitdir}/dnf-automatic.timer
%if %{with dnf5_obsoletes_dnf}
%{_bindir}/dnf-automatic
%else
%exclude %{_bindir}/dnf-automatic
%endif

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
    -DWITH_LIBDNF5_CLI=%{?with_libdnf_cli:ON}%{!?with_libdnf_cli:OFF} \
    -DWITH_DNF5=%{?with_dnf5:ON}%{!?with_dnf5:OFF} \
    -DWITH_PLUGIN_ACTIONS=%{?with_plugin_actions:ON}%{!?with_plugin_actions:OFF} \
    -DWITH_PLUGIN_RHSM=%{?with_plugin_rhsm:ON}%{!?with_plugin_rhsm:OFF} \
    -DWITH_PYTHON_PLUGINS_LOADER=%{?with_python_plugins_loader:ON}%{!?with_python_plugins_loader:OFF} \
    \
    -DWITH_COMPS=%{?with_comps:ON}%{!?with_comps:OFF} \
    -DWITH_MODULEMD=%{?with_modulemd:ON}%{!?with_modulemd:OFF} \
    -DWITH_ZCHUNK=%{?with_zchunk:ON}%{!?with_zchunk:OFF} \
    -DWITH_SYSTEMD=%{?with_systemd:ON}%{!?with_systemd:OFF} \
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

%if %{with dnf5_obsoletes_dnf}
ln -sr %{buildroot}%{_bindir}/dnf5 %{buildroot}%{_bindir}/dnf
ln -sr %{buildroot}%{_bindir}/dnf5 %{buildroot}%{_bindir}/yum
ln -sr %{buildroot}%{_datadir}/bash-completion/completions/dnf5 %{buildroot}%{_datadir}/bash-completion/completions/dnf
%endif

# own dirs and files that dnf5 creates on runtime
mkdir -p %{buildroot}%{_prefix}/lib/sysimage/dnf
for files in \
    groups.toml modules.toml nevras.toml packages.toml \
    system.toml transaction_history.sqlite \
    transaction_history.sqlite-shm \
    transaction_history.sqlite-wal userinstalled.toml
do
    touch %{buildroot}%{_prefix}/lib/sysimage/dnf/$files
done

# Remove if condition when Fedora 37 is EOL
%if 0%{?fedora} > 37 || 0%{?rhel} > 10
ln -sr %{buildroot}%{_bindir}/dnf5 %{buildroot}%{_bindir}/microdnf
%endif

%if %{with systemd}
mkdir -p %{buildroot}%{_unitdir}/system-update.target.wants/
pushd %{buildroot}%{_unitdir}/system-update.target.wants/
  ln -sr ../dnf5-offline-transaction.service
popd
%endif

%find_lang dnf5
%find_lang dnf5-plugin-automatic
%find_lang dnf5-plugin-builddep
%find_lang dnf5-plugin-changelog
%find_lang dnf5-plugin-config-manager
%find_lang dnf5-plugin-copr
%find_lang dnf5-plugin-needs-restarting
%find_lang dnf5-plugin-repoclosure
%find_lang dnf5daemon-client
%find_lang dnf5daemon-server
%find_lang libdnf5
%find_lang libdnf5-cli
%find_lang libdnf5-plugin-actions
%find_lang libdnf5-plugin-rhsm

%ldconfig_scriptlets

%changelog
* Tue Apr 02 2024 Packit Team <hello@packit.dev> - 5.1.16-1
- New upstream release 5.1.16

* Fri Mar 15 2024 Packit Team <hello@packit.dev> - 5.1.15-1
- New upstream release 5.1.15

* Fri Mar 01 2024 Packit Team <hello@packit.dev> - 5.1.14-1
- New upstream release 5.1.14

* Tue Feb 20 2024 Packit Team <hello@packit.dev> - 5.1.13-1
- New upstream release 5.1.13

* Fri Feb 09 2024 Packit Team <hello@packit.dev> - 5.1.12-1
- New upstream release 5.1.12

* Thu Jan 11 2024 Packit Team <hello@packit.dev> - 5.1.11-1
- New upstream release 5.1.11

* Tue Jan 02 2024 Packit Team <hello@packit.dev> - 5.1.10-1
- New upstream release 5.1.10

* Fri Dec 08 2023 Packit Team <hello@packit.dev> - 5.1.9-1
- New upstream release 5.1.9

* Fri Nov 24 2023 Packit Team <hello@packit.dev> - 5.1.8-1
- New upstream release 5.1.8

* Thu Nov 09 2023 Packit Team <hello@packit.dev> - 5.1.7-1
- New upstream release 5.1.7

* Thu Oct 26 2023 Packit Team <hello@packit.dev> - 5.1.6-1
- New upstream release 5.1.6

* Thu Oct 05 2023 Packit Team <hello@packit.dev> - 5.1.5-1
- New upstream release 5.1.5

* Mon Sep 18 2023 Packit Team <hello@packit.dev> - 5.1.4-1
- New upstream release 5.1.4

* Tue Sep 12 2023 Packit Team <hello@packit.dev> - 5.1.3-1
- New upstream release 5.1.3

* Wed Aug 16 2023 Packit Team <hello@packit.dev> - 5.1.2-1
- New upstream release 5.1.2

* Fri Aug 04 2023 Packit Team <hello@packit.dev> - 5.1.1-1
- New upstream release 5.1.1

* Mon Jul 17 2023 Packit Team <hello@packit.dev> - 5.1.0-1
- New upstream release 5.1.0

* Thu Jun 29 2023 Packit Team <hello@packit.dev> - 5.0.15-1
- New upstream release 5.0.15

* Wed Jun 14 2023 Packit Team <hello@packit.dev> - 5.0.14-1
- New upstream release 5.0.14

* Mon May 29 2023 Packit Team <hello@packit.dev> - 5.0.13-1
- New upstream release 5.0.13

* Thu May 25 2023 Packit Team <hello@packit.dev> - 5.0.12-1
- New upstream release 5.0.12

* Thu May 18 2023 Packit Team <hello@packit.dev> - 5.0.11-1
- New upstream release 5.0.11

* Tue May 09 2023 Packit Team <hello@packit.dev> - 5.0.10-1
- New upstream release 5.0.10

* Tue Apr 18 2023 Nicola Sella <nsella@redhat.com> - 5.0.9-1
- New upstream release 5.0.9

* Thu Apr 13 2023 Nicola Sella <nsella@redhat.com> - 5.0.8-1
- New upstream release 5.0.8

* Wed Mar 8 2023 Nicola Sella <nsella@redhat.com> - 5.0.7-1
- New upstream release 5.0.7

* Tue Feb 14 2023 Nicola Sella <nsella@redhat.com> - 5.0.6-1
- New upstream release 5.0.6

* Thu Jan 26 2023 Nicola Sella <nsella@redhat.com> - 5.0.5-1
- New upstream release 5.0.5

* Thu Jan 12 2023 Nicola Sella <nsella@redhat.com> - 5.0.4-1
- New upstream release 5.0.4

* Wed Jan 04 2023 Nicola Sella <nsella@redhat.com> - 5.0.3-1
- New upstream release 5.0.3

* Thu Dec 08 2022 Nicola Sella <nsella@redhat.com> - 5.0.2-1
- New upstream release 5.0.2

* Thu Nov 24 2022 Nicola Sella <nsella@redhat.com> - 5.0.1-1
- New upstream release 5.0.1

* Wed Nov 2 2022 Nicola Sella <nsella@redhat.com> - 5.0.0-2~pre
- New upstream pre release 5.0.0

* Mon Oct 31 2022 Nicola Sella <nsella@redhat.com> - 5.0.0-1~pre
- New upstream pre release 5.0.0

* Fri Sep 16 2022 Nicola Sella - <nsella@redhat.com> - 5.0.0-0~pre
- New upstream pre release 5.0.0
