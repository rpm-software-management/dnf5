Name:           Package-post-ok
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Successfully runs post scriptlet

%post -p <lua>
print("post scriptlet successfully done")

%files

%changelog
