Name:           Package-filetriggerin-fail
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Fails to run filetriggerin scriptlet on Package-install-file install

%filetriggerin -p <lua> -- /usr/lib/Triggers
error("filetriggerin scriptlet (Package-filetriggerin-fail) for Package-install-file install/update is failing", 1)

%files

%changelog
