Name:           consumer
Epoch:          0
Version:        1
Release:        1
Vendor:         dnf5-test

License:        Public Domain
URL:            http://example.com/

Summary:        A dummy package requiring a file only resolvable via filelists metadata
Requires:       /opt/dnf5-test/tool
BuildArch:      noarch

%description
A dummy package with a file-based dependency that can only be resolved once filelists
metadata (not loaded by default) is loaded, since the required path isn't one createrepo_c
promotes into primary.xml (see provider-1-1.spec).

%files

%changelog
