Name:           SuperRipper
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for autoremove testing.

Provides:       CQRlib = 1.1.2-16.fc29
Provides:       CQRlib(x86-64) = 1.1.2-16.fc29
Provides:       libCQRlib.so.2()(64bit)
BuildRequires:  %{?buildrequires}%{?!buildrequires:lame-libs}
Requires:       abcde

%description
This package in 1.0 version requires abcde. In 1.2 version is this dependency dropped.
It is used for testing autoremove command.

%files

%changelog
