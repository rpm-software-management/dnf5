Name:           Package-post-fail
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Fails on post scriptlet

%post -p <lua>
error("failing on post scriptlet", 1)

%files

%changelog
