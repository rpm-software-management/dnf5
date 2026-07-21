Name:           Package-posttrans-fail
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Fails on posttrans scriptlet

%posttrans -p <lua>
error("failing on posttrans scriptlet", 1)

%files

%changelog
