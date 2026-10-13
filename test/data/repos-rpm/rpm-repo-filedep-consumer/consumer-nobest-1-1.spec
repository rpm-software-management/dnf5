Name:           consumer-nobest
Epoch:          0
Version:        1
Release:        1
Vendor:         dnf5-test

License:        Public Domain
URL:            http://example.com/

Summary:        A dummy package with no requires, installable even without filelists
BuildArch:      noarch

%description
The older, always-installable version of a package that also exists in a newer version
(see consumer-nobest-2-1.spec) whose only difference is a file-based Requires - used to
exercise the --no-best silent-skip trigger for the automatic filelists retry.

%files

%changelog
