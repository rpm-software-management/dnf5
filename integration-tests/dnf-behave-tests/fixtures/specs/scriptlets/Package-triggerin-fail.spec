Name:           Package-triggerin-fail
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Fails on triggering scriptlet on Package-install-file install/update

%triggerin -p <lua> -- Package-install-file
error("failing on triggerin scriptlet", 1)

%files

%changelog
