Name:           basesystem
Epoch:          0
Version:        11
Release:        8.fc29

License:        Public Domain
URL:            None

Summary:        The skeleton package which defines a simple Fedora system
BuildArch:      noarch

Provides:       basesystem = 11-8.fc29

Requires:       filesystem
Requires:       setup

%description
Basesystem defines the components of a basic Fedora system
(for example, the package installation order to use during bootstrapping).
Basesystem should be in every installation of a system, and it
should never be removed.

%files

%changelog
