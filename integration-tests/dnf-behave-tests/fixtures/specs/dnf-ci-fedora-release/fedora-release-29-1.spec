Name:           fedora-release
Epoch:          0
Version:        29
Release:        1

License:        MIT
URL:            https://fedoraproject.org/

Summary:        Fedora release files
BuildArch:      noarch

Provides:       redhat-release
Provides:       system-release(29)
Provides:       system-release
Provides:       fedora-release = 29-1
Provides:       config(fedora-release) = 29-1

# detected $releasever should be '123'
Provides:       system-release(releasever) = 123
# $releasever_major, $releasever_minor should be overridden to 45, 67
Provides:       system-release(releasever_major) = 45
Provides:       system-release(releasever_minor) = 67


%description
Fedora release files such as various /etc/ files that define the release
and systemd preset files that determine which services are enabled by default.


%files

%changelog
