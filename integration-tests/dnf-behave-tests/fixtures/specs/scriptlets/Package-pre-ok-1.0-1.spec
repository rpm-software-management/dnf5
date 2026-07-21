Name:           Package-pre-ok
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Successfully runs pre scriptlet

%pre -p <lua>
print("pre scriptlet successfully done")

%files

%changelog
