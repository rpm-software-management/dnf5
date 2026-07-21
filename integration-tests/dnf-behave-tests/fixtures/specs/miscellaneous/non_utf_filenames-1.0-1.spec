Name:           non_utf_filenames
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        A package with non-utf8 filenames

BuildArch:      noarch

%description
Dummy.

%install
mkdir -p %{buildroot}/non-utf8-files/
pushd %{buildroot}/non-utf8-files/
  # Create dummy test files which names are not valid UTF-8 strings.
  # They contain characters with hex codes \x80 and \xa0 respectively.
  touch hex-80-€-
  touch hex-a0- -
popd

%files
%dir /non-utf8-files
/non-utf8-files/*

%changelog
