Name:           Package-postun-fail
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Fails on postun scriptlet

%postun -p <lua>
error("failing on postun scriptlet", 1)

%files

%changelog
