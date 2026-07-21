Name:           setup
Epoch:          0
Version:        2.14.2
Release:        1.fc29

License:        Public Domain
URL:            https://pagure.io/setup/

Summary:        A set of system configuration and setup files
BuildArch:      noarch

Provides:       config(setup) = 2.14.2-1.fc29
Provides:       setup = 2.14.2-1.fc29

#Requires:       system-release

Conflicts:      filesystem < 3
Conflicts:      bash <= 2.0.4-21
Conflicts:      initscripts < 4.26

%description
The setup package contains a set of important system configuration and
setup files, such as passwd, group, and profile.

%files

%changelog
