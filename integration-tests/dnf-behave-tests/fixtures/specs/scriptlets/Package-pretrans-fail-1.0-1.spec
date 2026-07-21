Name:           Package-pretrans-fail
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Fails on pretrans scriptlet

%pretrans -p <lua>
error("failing on pretrans scriptlet", 1)

%files

%changelog
