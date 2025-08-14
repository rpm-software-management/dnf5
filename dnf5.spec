%global project_version_prime 5
%global project_version_major 2
%global project_version_minor 16
%global project_version_micro 0

%bcond dnf5_obsoletes_dnf %[0%{?fedora} > 40 || 0%{?rhel} > 10]

Name:           dnf5
Version:        %{project_version_prime}.%{project_version_major}.%{project_version_minor}.%{project_version_micro}
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
Recommends:     (dnf5-plugins if dnf-plugins-core)
Recommends:     bash-completion
Requires:       coreutils
%if 0%{?fedora} > 41
Recommends:     (libdnf5-plugin-expired-pgp-keys if gnupg2)
%endif

%if 0%{?fedora} || 0%{?rhel} > 10
Provides:       microdnf = %{version}-%{release}
Obsoletes:      microdnf < 4
%endif

%if %{with dnf5_obsoletes_dnf}
Provides:       dnf = %{version}-%{release}
Obsoletes:      dnf < 5

Provides:       yum = %{version}-%{release}
Obsoletes:      yum < 5

Conflicts:      python3-dnf-plugins-core < 4.7.0
%endif

Provides:       dnf5-command(advisory)
Provides:       dnf5-command(autoremove)
Provides:       dnf5-command(check)
Provides:       dnf5-command(check-upgrade)
Provides:       dnf5-command(clean)
Provides:       dnf5-command(distro-sync)
Provides:       dnf5-command(downgrade)
Provides:       dnf5-command(download)
Provides:       dnf5-command(environment)
Provides:       dnf5-command(group)
Provides:       dnf5-command(history)
Provides:       dnf5-command(info)
Provides:       dnf5-command(install)
Provides:       dnf5-command(leaves)
Provides:       dnf5-command(list)
Provides:       dnf5-command(makecache)
Provides:       dnf5-command(mark)
Provides:       dnf5-command(module)
Provides:       dnf5-command(offline)
Provides:       dnf5-command(provides)
Provides:       dnf5-command(reinstall)
Provides:       dnf5-command(replay)
Provides:       dnf5-command(remove)
Provides:       dnf5-command(repo)
Provides:       dnf5-command(repoquery)
Provides:       dnf5-command(search)
Provides:       dnf5-command(swap)
Provides:       dnf5-command(system-upgrade)
Provides:       dnf5-command(upgrade)
Provides:       dnf5-command(versionlock)


# ========== build options ==========

%bcond_without dnf5daemon_client
%bcond_without dnf5daemon_server
%bcond_without libdnf_cli
%bcond_without dnf5
%bcond_without dnf5_plugins
%bcond_without plugin_actions
%bcond_without plugin_appstream
%bcond_without plugin_expired_pgp_keys
%bcond_without plugin_rhsm
%bcond_without python_plugins_loader

%bcond_without comps
%bcond_without modulemd
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

# Disable SOLVER_FLAG_FOCUS_NEW only for RHEL
%if 0%{?rhel} && 0%{?rhel} < 11
%bcond_with    focus_new
%else
%bcond_without focus_new
%endif

%if %{with clang}
    %global toolchain clang
%endif

# ========== versions of dependencies ==========

%global libmodulemd_version 2.5.0
%global librepo_version 1.20.0
%if %{with focus_new}
    %global libsolv_version 0.7.30
%else
    %global libsolv_version 0.7.25
%endif
%global sqlite_version 3.35.0
%global swig_version 4


# ========== build requires ==========

%if 0%{?fedora} > 40 || 0%{?rhel} > 10
BuildRequires:  bash-completion-devel
%else
BuildRequires:  bash-completion
%endif
BuildRequires:  cmake >= 3.21
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
BuildRequires:  zlib-devel

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
BuildRequires:  perl(FindBin)
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

%post
%if %{with dnf5_obsoletes_dnf}
%systemd_post dnf-makecache.timer
%else
%systemd_post dnf5-makecache.timer
%endif

%preun
%if %{with dnf5_obsoletes_dnf}
%systemd_preun dnf-makecache.timer
%else
%systemd_preun dnf5-makecache.timer
%endif

%postun
%if %{with dnf5_obsoletes_dnf}
%systemd_postun_with_restart dnf-makecache.timer
%else
%systemd_postun_with_restart dnf5-makecache.timer
%endif

%files -f dnf5.lang
%{_bindir}/dnf5
%if %{with dnf5_obsoletes_dnf}
%{_bindir}/dnf
%{_bindir}/yum
%endif
%{_unitdir}/dnf*-makecache.service
%{_unitdir}/dnf*-makecache.timer

%if 0%{?fedora} || 0%{?rhel} > 10
%{_bindir}/microdnf
%endif

%dir %{_sysconfdir}/dnf/dnf5-aliases.d
%doc %{_sysconfdir}/dnf/dnf5-aliases.d/README
%dir %{_datadir}/dnf5
%dir %{_datadir}/dnf5/aliases.d
%{_datadir}/dnf5/aliases.d/compatibility.conf
%dir %{_libdir}/dnf5
%dir %{_libdir}/dnf5/plugins
%dir %{_datadir}/dnf5/dnf5-plugins
%dir %{_sysconfdir}/dnf/dnf5-plugins
%doc %{_libdir}/dnf5/plugins/README
%dir %{_libdir}/libdnf5/plugins
%dir %{_datadir}/bash-completion/
%dir %{_datadir}/bash-completion/completions/
%{_datadir}/bash-completion/completions/dnf*
%license COPYING.md
%license gpl-2.0.txt
%doc AUTHORS.md CHANGELOG.md CONTRIBUTING.md README.md
%if %{with man}
%{_mandir}/man8/dnf5.8.*
%if %{with dnf5_obsoletes_dnf}
%{_mandir}/man8/dnf.8.*
%endif
%{_mandir}/man8/dnf*-advisory.8.*
%{_mandir}/man8/dnf*-autoremove.8.*
%{_mandir}/man8/dnf*-check.8.*
%{_mandir}/man8/dnf*-check-upgrade.8.*
%{_mandir}/man8/dnf*-clean.8.*
%{_mandir}/man8/dnf*-distro-sync.8.*
%{_mandir}/man8/dnf*-downgrade.8.*
%{_mandir}/man8/dnf*-download.8.*
%{_mandir}/man8/dnf*-environment.8.*
%{_mandir}/man8/dnf*-group.8.*
%{_mandir}/man8/dnf*-history.8.*
%{_mandir}/man8/dnf*-info.8.*
%{_mandir}/man8/dnf*-install.8.*
%{_mandir}/man8/dnf*-leaves.8.*
%{_mandir}/man8/dnf*-list.8.*
%{_mandir}/man8/dnf*-makecache.8.*
%{_mandir}/man8/dnf*-mark.8.*
%{_mandir}/man8/dnf*-module.8.*
%{_mandir}/man8/dnf*-offline.8.*
%{_mandir}/man8/dnf*-provides.8.*
%{_mandir}/man8/dnf*-reinstall.8.*
%{_mandir}/man8/dnf*-remove.8.*
%{_mandir}/man8/dnf*-replay.8.*
%{_mandir}/man8/dnf*-repo.8.*
%{_mandir}/man8/dnf*-repoquery.8.*
%{_mandir}/man8/dnf*-search.8.*
%{_mandir}/man8/dnf*-swap.8.*
%{_mandir}/man8/dnf*-system-upgrade.8.*
%{_mandir}/man8/dnf*-upgrade.8.*
%{_mandir}/man8/dnf*-versionlock.8.*
%{_mandir}/man7/dnf*-aliases.7.*
%{_mandir}/man7/dnf*-caching.7.*
%{_mandir}/man7/dnf*-comps.7.*
%{_mandir}/man7/dnf*-filtering.7.*
%{_mandir}/man7/dnf*-forcearch.7.*
%{_mandir}/man7/dnf*-installroot.7.*
%{_mandir}/man7/dnf*-modularity.7.*
%{_mandir}/man7/dnf*-specs.7.*
%{_mandir}/man7/dnf*-system-state.7.*
%{_mandir}/man7/dnf*-changes-from-dnf4.7.*
%{_mandir}/man5/dnf*.conf.5.*
%{_mandir}/man5/dnf*.conf-todo.5.*
%{_mandir}/man5/dnf*.conf-deprecated.5.*
%endif

%if %{with systemd}
%{_unitdir}/dnf5-offline-transaction.service
%{_unitdir}/dnf5-offline-transaction-cleanup.service
%{_unitdir}/system-update.target.wants/dnf5-offline-transaction.service
%endif

%if %{without dnf5_plugins}
%exclude %{_datadir}/dnf5/aliases.d/compatibility-plugins.conf
%exclude %{_datadir}/dnf5/aliases.d/compatibility-reposync.conf
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
Conflicts:      dnf-data < 4.20.0
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
%ghost %attr(0644, root, root) %{_sysconfdir}/dnf/versionlock.toml
%dir %{_datadir}/dnf5/libdnf.conf.d
%dir %{_sysconfdir}/dnf/libdnf5.conf.d
%dir %{_datadir}/dnf5/repos.override.d
%dir %{_sysconfdir}/dnf/repos.override.d
%dir %{_sysconfdir}/dnf/libdnf5-plugins
%dir %{_datadir}/dnf5/repos.d
%dir %{_datadir}/dnf5/vars.d
%dir %{_libdir}/libdnf5
%{_libdir}/libdnf5.so.2*
%dir %{_prefix}/lib/sysimage/libdnf5
%attr(0755, root, root) %ghost %dir %{_prefix}/lib/sysimage/libdnf5/comps_groups
%verify(not md5 size mtime) %attr(0644, root, root) %ghost %{_prefix}/lib/sysimage/libdnf5/environments.toml
%verify(not md5 size mtime) %attr(0644, root, root) %ghost %{_prefix}/lib/sysimage/libdnf5/groups.toml
%verify(not md5 size mtime) %attr(0644, root, root) %ghost %{_prefix}/lib/sysimage/libdnf5/modules.toml
%verify(not md5 size mtime) %attr(0644, root, root) %ghost %{_prefix}/lib/sysimage/libdnf5/nevras.toml
%attr(0755, root, root) %ghost %dir %{_prefix}/lib/sysimage/libdnf5/offline
%verify(not md5 size mtime) %attr(0644, root, root) %ghost %{_prefix}/lib/sysimage/libdnf5/offline/offline-transaction-state.toml
%attr(0755, root, root) %ghost %dir %{_prefix}/lib/sysimage/libdnf5/offline/packages
%verify(not md5 size mtime) %attr(0644, root, root) %ghost %{_prefix}/lib/sysimage/libdnf5/offline/transaction.json
%verify(not md5 size mtime) %attr(0644, root, root) %ghost %{_prefix}/lib/sysimage/libdnf5/packages.toml
%verify(not md5 size mtime) %attr(0644, root, root) %ghost %{_prefix}/lib/sysimage/libdnf5/system.toml
%verify(not md5 size mtime) %attr(0644, root, root) %ghost %{_prefix}/lib/sysimage/libdnf5/transaction_history.sqlite{,-shm,-wal}
%license lgpl-2.1.txt
%ghost %attr(0755, root, root) %dir %{_var}/cache/libdnf5
%ghost %attr(0755, root, root) %dir %{_sharedstatedir}/dnf

# ========== libdnf5-cli ==========

%if %{with libdnf_cli}
%package -n libdnf5-cli
Summary:        Library for working with a terminal in a command-line package manager
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}

%description -n libdnf5-cli
Library for working with a terminal in a command-line package manager.

%files -n libdnf5-cli -f libdnf5-cli.lang
%{_libdir}/libdnf5-cli.so.2*
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
Summary:        Python 3 bindings for the libdnf5 library
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
%if %{with man}
%{_mandir}/man8/libdnf5-actions.8.*
%endif
%endif

# ========== libdnf5-plugin-appstream ==========

%if %{with plugin_appstream}

%package -n libdnf5-plugin-appstream
Summary:        Libdnf5 plugin to install repository AppStream data
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}
BuildRequires:  pkgconfig(appstream) >= 0.16

%description -n libdnf5-plugin-appstream
Libdnf5 plugin that installs repository's AppStream data, for repositories which provide them.

%files -n libdnf5-plugin-appstream
%{_libdir}/libdnf5/plugins/appstream.so
%config %{_sysconfdir}/dnf/libdnf5-plugins/appstream.conf

%endif

# ========== libdnf5-plugin-expired-pgp-keys ==========

%if %{with plugin_expired_pgp_keys}
%package -n libdnf5-plugin-expired-pgp-keys
Summary:        Libdnf5 plugin for detecting and removing expired PGP keys
License:        LGPL-2.1-or-later
Requires:       libdnf5%{?_isa} = %{version}-%{release}
Requires:       gnupg2
%if 0%{?fedora} >= 43 || 0%{?rhel} >= 11
Requires:       rpm-libs%{?_isa} >= 5.99.90
%endif

%description -n libdnf5-plugin-expired-pgp-keys
Libdnf5 plugin for detecting and removing expired PGP keys.

%files -n libdnf5-plugin-expired-pgp-keys -f libdnf5-plugin-expired-pgp-keys.lang
%{_libdir}/libdnf5/plugins/expired-pgp-keys.*
%config %{_sysconfdir}/dnf/libdnf5-plugins/expired-pgp-keys.conf
%if %{with man}
%{_mandir}/man8/libdnf5-expired-pgp-keys.8.*
%endif
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
%if %{with man}
%{_mandir}/man8/dnf5daemon-client.8.*
%endif
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
%config(noreplace) %{_sysconfdir}/dnf/dnf5daemon-server.conf
%{_sbindir}/dnf5daemon-server
%{_unitdir}/dnf5daemon-server.service
%{_datadir}/dbus-1/system.d/org.rpm.dnf.v0.conf
%{_datadir}/dbus-1/system-services/org.rpm.dnf.v0.service
%{_datadir}/dbus-1/interfaces/org.rpm.dnf.v0.*.xml
%{_datadir}/polkit-1/actions/org.rpm.dnf.v0.policy
%license COPYING.md
%license gpl-2.0.txt
%if %{with man}
%{_mandir}/man8/dnf5daemon-server.8.*
%{_mandir}/man8/dnf5daemon-dbus-api.8.*
%endif


# ========== dnf5daemon-server-polkit ==========

%package -n dnf5daemon-server-polkit
Summary:        Polkit rule to allow wheel group members install trusted packages
License:        GPL-2.0-or-later
Requires:       polkit
Requires:       dnf5daemon-server = %{version}-%{release}
BuildArch:      noarch

%description -n dnf5daemon-server-polkit
Polkit rule to allow active local wheel group members install packages from
trusted repositories using dnf5daemon-server.

%files -n dnf5daemon-server-polkit
%{_datadir}/polkit-1/rules.d/org.rpm.dnf.v0.rules
%endif


# ========== dnf5-plugins ==========
%if %{with dnf5_plugins}

%package -n dnf5-plugins
Summary:        Plugins for dnf5
License:        LGPL-2.1-or-later
Requires:       dnf5%{?_isa} = %{version}-%{release}
Requires:       libcurl%{?_isa} >= 7.62.0
Requires:       libdnf5%{?_isa} = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
Provides:       dnf5-command(builddep)
Provides:       dnf5-command(changelog)
Provides:       dnf5-command(config-manager)
Provides:       dnf5-command(copr)
Provides:       dnf5-command(needs-restarting)
Provides:       dnf5-command(repoclosure)
Provides:       dnf5-command(reposync)
Provides:       dnf5-command(repomanage)

%description -n dnf5-plugins
Core DNF5 plugins that enhance dnf5 with builddep, changelog,
config-manager, copr, repoclosure, repomanage and reposync commands.

%files -n dnf5-plugins -f dnf5-plugin-builddep.lang -f dnf5-plugin-changelog.lang -f dnf5-plugin-config-manager.lang -f dnf5-plugin-copr.lang -f dnf5-plugin-needs-restarting.lang -f dnf5-plugin-repoclosure.lang -f dnf5-plugin-reposync.lang
%{_libdir}/dnf5/plugins/builddep_cmd_plugin.so
%{_libdir}/dnf5/plugins/changelog_cmd_plugin.so
%{_libdir}/dnf5/plugins/config-manager_cmd_plugin.so
%{_libdir}/dnf5/plugins/copr_cmd_plugin.so
%{_libdir}/dnf5/plugins/needs_restarting_cmd_plugin.so
%{_libdir}/dnf5/plugins/repoclosure_cmd_plugin.so
%{_libdir}/dnf5/plugins/reposync_cmd_plugin.so
%{_libdir}/dnf5/plugins/repomanage_cmd_plugin.so
%if %{with man}
%{_mandir}/man8/dnf*-builddep.8.*
%{_mandir}/man8/dnf*-changelog.8.*
%{_mandir}/man8/dnf*-config-manager.8.*
%{_mandir}/man8/dnf*-copr.8.*
%{_mandir}/man8/dnf*-needs-restarting.8.*
%{_mandir}/man8/dnf*-repoclosure.8.*
%{_mandir}/man8/dnf*-reposync.8.*
%{_mandir}/man8/dnf*-repomanage.8.*
%endif
%{_datadir}/dnf5/aliases.d/compatibility-plugins.conf
%{_datadir}/dnf5/aliases.d/compatibility-reposync.conf


# ========== dnf5-automatic plugin ==========

%package plugin-automatic
Summary:        Package manager - automated upgrades
License:        LGPL-2.1-or-later
Requires:       dnf5%{?_isa} = %{version}-%{release}
Requires:       libcurl-full%{?_isa}
Requires:       libdnf5%{?_isa} = %{version}-%{release}
Requires:       libdnf5-cli%{?_isa} = %{version}-%{release}
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
%ghost %attr(0644, root, root) %{_sysconfdir}/motd.d/dnf5-automatic
%{_libdir}/dnf5/plugins/automatic_cmd_plugin.so
%{_datadir}/dnf5/dnf5-plugins/automatic.conf
%ghost %attr(0644, root, root) %config(noreplace) %{_sysconfdir}/dnf/automatic.conf
%ghost %attr(0644, root, root) %config(noreplace) %{_sysconfdir}/dnf/dnf5-plugins/automatic.conf
%if %{with man}
%{_mandir}/man8/dnf*-automatic.8.*
%endif
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
    -DENABLE_SOLV_FOCUSNEW=%{?with_focus_new:ON}%{!?with_focus_new:OFF} \
    \
    -DWITH_DNF5DAEMON_CLIENT=%{?with_dnf5daemon_client:ON}%{!?with_dnf5daemon_client:OFF} \
    -DWITH_DNF5DAEMON_SERVER=%{?with_dnf5daemon_server:ON}%{!?with_dnf5daemon_server:OFF} \
    -DWITH_LIBDNF5_CLI=%{?with_libdnf_cli:ON}%{!?with_libdnf_cli:OFF} \
    -DWITH_DNF5=%{?with_dnf5:ON}%{!?with_dnf5:OFF} \
    -DWITH_DNF5_OBSOLETES_DNF=%{?with_dnf5_obsoletes_dnf:ON}%{!?with_dnf5_obsoletes_dnf:OFF} \
    -DWITH_DNF5_PLUGINS=%{?with_dnf5_plugins:ON}%{!?with_dnf5_plugins:OFF} \
    -DWITH_PLUGIN_ACTIONS=%{?with_plugin_actions:ON}%{!?with_plugin_actions:OFF} \
    -DWITH_PLUGIN_APPSTREAM=%{?with_plugin_appstream:ON}%{!?with_plugin_appstream:OFF} \
    -DWITH_PLUGIN_EXPIRED_PGP_KEYS=%{?with_plugin_expired_pgp_keys:ON}%{!?with_plugin_expired_pgp_keys:OFF} \
    -DWITH_PLUGIN_RHSM=%{?with_plugin_rhsm:ON}%{!?with_plugin_rhsm:OFF} \
    -DWITH_PYTHON_PLUGINS_LOADER=%{?with_python_plugins_loader:ON}%{!?with_python_plugins_loader:OFF} \
    \
    -DWITH_COMPS=%{?with_comps:ON}%{!?with_comps:OFF} \
    -DWITH_MODULEMD=%{?with_modulemd:ON}%{!?with_modulemd:OFF} \
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
    -DVERSION_PRIME=%{project_version_prime} \
    -DVERSION_MAJOR=%{project_version_major} \
    -DVERSION_MINOR=%{project_version_minor} \
    -DVERSION_MICRO=%{project_version_micro}
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
%if %{with man}
    for file in %{buildroot}%{_mandir}/man[578]/dnf5[-.]*; do
        dir=$(dirname $file)
        filename=$(basename $file)
        ln -sr $file $dir/${filename/dnf5/dnf}
    done
%endif
# Make "dnf-makecache" the "real" unit name, but keep compatibility for playbooks that refer to dnf5-makecache
mv %{buildroot}%{_unitdir}/dnf5-makecache.service %{buildroot}%{_unitdir}/dnf-makecache.service
mv %{buildroot}%{_unitdir}/dnf5-makecache.timer %{buildroot}%{_unitdir}/dnf-makecache.timer
ln -s dnf-makecache.service %{buildroot}%{_unitdir}/dnf5-makecache.service
ln -s dnf-makecache.timer %{buildroot}%{_unitdir}/dnf5-makecache.timer
%endif

# own dirs and files that dnf5 creates on runtime
mkdir -p %{buildroot}%{_prefix}/lib/sysimage/libdnf5
for file in \
    environments.toml groups.toml modules.toml nevras.toml packages.toml \
    system.toml \
    transaction_history.sqlite transaction_history.sqlite-shm \
    transaction_history.sqlite-wal
do
    touch %{buildroot}%{_prefix}/lib/sysimage/libdnf5/$file
done
mkdir -p %{buildroot}%{_prefix}/lib/sysimage/libdnf5/comps_groups
mkdir -p %{buildroot}%{_prefix}/lib/sysimage/libdnf5/offline
touch %{buildroot}%{_sysconfdir}/dnf/versionlock.toml

%if 0%{?fedora} || 0%{?rhel} > 10
ln -sr %{buildroot}%{_bindir}/dnf5 %{buildroot}%{_bindir}/microdnf
%endif

%if %{with systemd}
mkdir -p %{buildroot}%{_unitdir}/system-update.target.wants/
pushd %{buildroot}%{_unitdir}/system-update.target.wants/
  ln -sr ../dnf5-offline-transaction.service
popd
%endif

mkdir -p %{buildroot}%{_libdir}/libdnf5/plugins

%find_lang dnf5
%if %{with dnf5_plugins}
%find_lang dnf5-plugin-automatic
%find_lang dnf5-plugin-builddep
%find_lang dnf5-plugin-changelog
%find_lang dnf5-plugin-config-manager
%find_lang dnf5-plugin-copr
%find_lang dnf5-plugin-needs-restarting
%find_lang dnf5-plugin-repoclosure
%find_lang dnf5-plugin-reposync
%endif
%if %{with dnf5daemon_client}
%find_lang dnf5daemon-client
%endif
%if %{with dnf5daemon_server}
%find_lang dnf5daemon-server
%endif
%find_lang libdnf5
%if %{with libdnf_cli}
%find_lang libdnf5-cli
%endif
%if %{with plugin_actions}
%find_lang libdnf5-plugin-actions
%endif
%if %{with plugin_expired_pgp_keys}
%find_lang libdnf5-plugin-expired-pgp-keys
%endif
%if %{with plugin_rhsm}
%find_lang libdnf5-plugin-rhsm
%endif

%ldconfig_scriptlets

%changelog
* Thu Aug 07 2025 Packit Team <hello@packit.dev> - 5.2.16.0-1
- New upstream release 5.2.16.0

* Fri Jul 11 2025 Packit Team <hello@packit.dev> - 5.2.15.0-1
- New upstream release 5.2.15.0

* Fri Jun 20 2025 Packit Team <hello@packit.dev> - 5.2.14.0-1
- New upstream release 5.2.14.0

* Wed Apr 23 2025 Packit Team <hello@packit.dev> - 5.2.13.1-1
- New upstream release 5.2.13.1

* Mon Apr 21 2025 Packit Team <hello@packit.dev> - 5.2.13.0-1
- New upstream release 5.2.13.0

* Tue Mar 18 2025 Packit Team <hello@packit.dev> - 5.2.12.0-1
- New upstream release 5.2.12.0

* Fri Mar 07 2025 Packit Team <hello@packit.dev> - 5.2.11.0-1
- New upstream release 5.2.11.0

* Thu Feb 06 2025 Packit Team <hello@packit.dev> - 5.2.10.0-1
- New upstream release 5.2.10.0

* Tue Feb 04 2025 Packit Team <hello@packit.dev> - 5.2.9.0-1
- New upstream release 5.2.9.0

* Thu Dec 05 2024 Packit Team <hello@packit.dev> - 5.2.8.1-1
- New upstream release 5.2.8.1

* Mon Dec 02 2024 Packit Team <hello@packit.dev> - 5.2.8.0-1
- New upstream release 5.2.8.0

* Tue Nov 12 2024 Packit Team <hello@packit.dev> - 5.2.7.0-1
- New upstream release 5.2.7.0

* Fri Sep 20 2024 Packit Team <hello@packit.dev> - 5.2.6.2-1
- New upstream release 5.2.6.2

* Thu Sep 19 2024 Packit Team <hello@packit.dev> - 5.2.6.1-1
- New upstream release 5.2.6.1

* Mon Sep 09 2024 Packit Team <hello@packit.dev> - 5.2.6.0-1
- New upstream release 5.2.6.0

* Tue Jul 23 2024 Packit Team <hello@packit.dev> - 5.2.5.0-1
- New upstream release 5.2.5.0

* Wed Jun 26 2024 Packit Team <hello@packit.dev> - 5.2.4.0-1
- New upstream release 5.2.4.0

* Mon Jun 03 2024 Packit Team <hello@packit.dev> - 5.2.3.0-1
- New upstream release 5.2.3.0

* Tue May 28 2024 Packit Team <hello@packit.dev> - 5.2.2.0-1
- New upstream release 5.2.2.0

* Mon May 06 2024 Packit Team <hello@packit.dev> - 5.2.1.0-1
- New upstream release 5.2.1.0

* Wed Apr 24 2024 Packit Team <hello@packit.dev> - 5.2.0.0-1
- New upstream release 5.2.0.0

* Wed Apr 03 2024 Packit Team <hello@packit.dev> - 5.1.17-1
- New upstream release 5.1.17

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
