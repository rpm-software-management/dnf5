Name:           Package-filetriggerin-ok
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Successfully runs filetriggerin scriptlet on Package-install-file install

%filetriggerin -p <lua> -- /usr/lib/Triggers
print("filetriggerin scriptlet (Package-filetriggerin-ok) for Package-install-file install/update successfully done")

%files

%changelog
