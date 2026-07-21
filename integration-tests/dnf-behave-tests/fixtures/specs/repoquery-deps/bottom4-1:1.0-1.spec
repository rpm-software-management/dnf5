Name:           bottom4
Epoch:          1
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Provides:       bottom4-prov1

Summary:        Bottom level package (other packages depend on it).

%description
Dummy.

%install
mkdir -p %{buildroot}/a
touch %{buildroot}/a/bottom4-file

%files
/a/bottom4-file

%changelog
