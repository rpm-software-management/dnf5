Name:           Package-pre-fail
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Fails on pre scriptlet

%pre -p <lua>
error("failing on pre scriptlet", 1)

%files

%changelog
