Name:           consumer-missing
Epoch:          0
Version:        1
Release:        1
Vendor:         dnf5-test

License:        Public Domain
URL:            http://example.com/

Summary:        A dummy package requiring a file no package in either repo provides
Requires:       /opt/dnf5-test/does-not-exist
BuildArch:      noarch

%description
A dummy package with a file-based dependency that no package provides, used to verify the
automatic filelists retry gives up cleanly (a single attempt, no loop) when loading
filelists doesn't actually resolve the problem.

%files

%changelog
