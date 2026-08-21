Name:           Package-preun-fail
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Fails on preun scriptlet

%preun -p <lua>
error("failing on preun scriptlet", 1)

%files

%changelog
