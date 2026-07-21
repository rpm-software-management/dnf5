# This package is not meant to be built, it's for installing the test suite
# dependencies.
#
# Run the following command to install the dependencies:
# $ dnf builddep ci-dnf-stack.spec
#
# Some of the dependencies may not be available as RPMs on the target system.
# We use pip for those:
# $ pip3 install -r requirements.txt

Name:           dnf-integration-test-suite-requirements
Version:        1
Release:        1
Summary:        Requirements for the DNF Integration Test Suite.
License:        GPLv3


# test suite dependencies
BuildRequires:  acl
# attr for "getfattr" command
BuildRequires:  attr
# coreutils for "chmod", "head', "id", "ln", "ls", "mv", "sha256sum", "sleep",
# "sort", "stat", "touch" commands
BuildRequires:  coreutils
BuildRequires:  createrepo_c
# diffutils for "diff" command
BuildRequires:  diffutils
BuildRequires:  fakeuname
BuildRequires:  findutils
BuildRequires:  glibc-langpack-en
BuildRequires:  glibc-langpack-de
BuildRequires:  grep
BuildRequires:  gzip
BuildRequires:  libfaketime
BuildRequires:  openssl
# psmisc for "killall" command
BuildRequires:  psmisc
BuildRequires:  python3 >= 3.11
BuildRequires:  python3-distro
BuildRequires:  python3-pip
BuildRequires:  python3-rpm
# a missing dep of python3-pip on f35 beta, remove when unneeded
BuildRequires:  python3-setuptools
BuildRequires:  rpm
BuildRequires:  rpm-build
BuildRequires:  rpm-sign
BuildRequires:  sed
# shadow-utils for "useradd" command
BuildRequires:  shadow-utils
BuildRequires:  sqlite
# util-linux for "su" command
BuildRequires:  util-linux
BuildRequires:  yq
BuildRequires:  zstd
%if 0%{?fedora}
BuildRequires:  python3-behave
BuildRequires:  python3-pexpect
BuildRequires:  zchunk
%endif

# dnfdaemon
BuildRequires:  dbus-daemon
BuildRequires:  python3-dbus
BuildRequires:  polkit

BuildRequires:  dnf5
BuildRequires:  dnf5-plugins
BuildRequires:  dnf5-plugin-automatic
BuildRequires:  dnf5-plugin-manifest
BuildRequires:  dnf5daemon-server
BuildRequires:  dnf5daemon-client
BuildRequires:  libdnf5-plugin-actions
%if ! ( 0%{?rhel} >= 10 )
BuildRequires:  libdnf5-plugin-local
%endif
BuildRequires:  libdnf5-plugin-expired-pgp-keys
BuildRequires:  python3-libdnf5-python-plugins-loader
# dnf5 python api tests need libdnf5 python bindings
BuildRequires:  python3-libdnf5

# debugging tools (always installed for simplicity)
BuildRequires: less
BuildRequires: openssh-clients
BuildRequires: procps-ng
BuildRequires: psmisc
BuildRequires: strace
BuildRequires: tcpdump
BuildRequires: vim-enhanced
BuildRequires: wget

%description
Requirements for the DNF Integration Test Suite.

%files

%changelog
