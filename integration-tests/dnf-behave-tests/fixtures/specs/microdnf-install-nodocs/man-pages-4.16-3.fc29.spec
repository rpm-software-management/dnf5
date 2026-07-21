Name: man-pages
Epoch:          0
Version: 4.16
Release: 3%{?dist}

License:        Public Domain
URL:            None

Summary: Linux kernel and C library user-space interface documentation

%description
Test manpage package.

%install
mkdir -p %{buildroot}/usr/share/doc/man-pages
touch %{buildroot}/usr/share/doc/man-pages/README

%files
%dir /usr/share/doc/man-pages
%doc README
/usr/share/doc/man-pages/README

%changelog
