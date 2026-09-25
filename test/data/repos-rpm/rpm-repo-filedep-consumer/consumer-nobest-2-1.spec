Name:           consumer-nobest
Epoch:          0
Version:        2
Release:        1
Vendor:         dnf5-test

License:        Public Domain
URL:            http://example.com/

Summary:        A newer version requiring a file only resolvable via filelists metadata
Requires:       /opt/dnf5-test/tool
BuildArch:      noarch

%description
The newer version of a package that would normally be preferred, but has a file-based
Requires that's unresolvable without filelists metadata (see provider-1-1.spec). With
best=false, the solver silently falls back to the older, requires-free version
(consumer-nobest-1-1.spec) instead of erroring - used to exercise the --no-best
silent-skip trigger for the automatic filelists retry.

%files

%changelog
