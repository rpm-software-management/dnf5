Name:           package-one
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package to fail on a file conflict

%description
File conflict package 1.

%install
mkdir -p %{buildroot}/usr/lib/package
echo 1 > %{buildroot}/usr/lib/package/conflicting-file

%files
%dir /usr/lib/package
/usr/lib/package/conflicting-file

%changelog
