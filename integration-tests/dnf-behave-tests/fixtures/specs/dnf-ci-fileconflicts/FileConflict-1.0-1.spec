Name:           FileConflict
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package to fail on file conflict

%description
File conflict package.

%install
mkdir -p %{buildroot}/usr/lib/FileConflict
mkdir -p %{buildroot}/usr/lib/FileConflict.bundled/a_dir
touch %{buildroot}/usr/lib/FileConflict.bundled/a_dir/a_file
ln -s ../FileConflict.bundled/a_dir %{buildroot}/usr/lib/FileConflict/a_dir

%files
%dir /usr/lib/FileConflict.bundled
%dir /usr/lib/FileConflict.bundled/a_dir
%dir /usr/lib/FileConflict
/usr/lib/FileConflict.bundled/a_dir/a_file
/usr/lib/FileConflict.bundled/a_dir/a_file
/usr/lib/FileConflict/a_dir

%changelog
