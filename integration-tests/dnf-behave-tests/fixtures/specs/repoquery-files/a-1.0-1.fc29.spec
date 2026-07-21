Name: a
Version: 1.0
Release: 1.fc29
Summary: Made up package

License: GPLv3+
Url: None

%description
a description

%install
mkdir -p %{buildroot}%{_bindir}
touch %{buildroot}%{_bindir}/a-binary
touch %{buildroot}/root-file

%files
%{_bindir}/a-binary
/root-file

%changelog
