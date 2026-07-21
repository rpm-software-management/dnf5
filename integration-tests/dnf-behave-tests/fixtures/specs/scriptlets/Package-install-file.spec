Name:           Package-install-file
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for triggering scriptlets.

%description
Installs file to trigger rpm file triggers

%install
mkdir -p %{buildroot}/usr/lib/Triggers
touch %{buildroot}/usr/lib/Triggers/trigger

%files
%dir /usr/lib/Triggers
/usr/lib/Triggers/trigger

%changelog
