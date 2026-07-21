Name:           fedora-release
Epoch:          0
Version:        30
Release:        1

License:        MIT
URL:            https://fedoraproject.org/

Summary:        Fedora release files
BuildArch:      noarch

Provides:       redhat-release
Provides:       system-release(30)
Provides:       system-release
Provides:       fedora-release = 30-1
Provides:       config(fedora-release) = 30-1
Provides:       base-module(pseudoplatform:7.0)


%description
Fedora release files such as various /etc/ files that define the release
and systemd preset files that determine which services are enabled by default.


%files

%changelog
