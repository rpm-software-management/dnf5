Name:           dnf-ci-kernel
Version:        3.0
Release:        1

License:        Public Domain
URL:            None
Summary:        Dummy

Provides:       installonlypkg(dnf-ci-kernel)
Provides:       /boot/vmlinuz-3.0
Requires:       dnf-ci-systemd

%description
Dummy.

%files
%ghost /boot/vmlinuz-3.0

%changelog
