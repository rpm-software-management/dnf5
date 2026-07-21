Name:           filesystem
Epoch:          0
Version:        3.9
Release:        2.fc29

License:        Public Domain
URL:            https://pagure.io/filesystem

Summary:        The basic directory layout for a Linux system

Provides:       filesystem = 3.9-2.fc29
Provides:       filesystem(x86-64) = 3.9-2.fc29

#Requires:       /bin/sh
Requires:       setup

%description
The filesystem package is one of the basic packages that is installed
on a Linux system. Filesystem contains the basic directory layout
for a Linux operating system, including the correct permissions for
the directories.

%package content
Summary:        Directory ownership content of the filesystem package

Provides:       filesystem-content = 3.9-2.fc29
Provides:       filesystem-content(x86-64) = 3.9-2.fc29

%description content
This subpackage of filesystem package contains just the file with
the directories owned by the filesystem package. This can be used
during the build process instead of calling rpm -ql filesystem.

%files

%files content

%changelog
