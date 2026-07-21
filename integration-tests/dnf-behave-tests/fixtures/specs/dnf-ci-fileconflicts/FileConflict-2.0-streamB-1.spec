Name:           FileConflict
Epoch:          0
Version:        2.0.streamB
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for modularity testing

%description
Modularity testing package.

%install
mkdir -p %{buildroot}/usr/lib/FileConflict/a_dir
touch %{buildroot}/usr/lib/FileConflict/a_dir/a_file

%files
%dir /usr/lib/FileConflict
%dir /usr/lib/FileConflict/a_dir
/usr/lib/FileConflict/a_dir/a_file

%changelog
