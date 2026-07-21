%undefine _debuginfo_subpackages

Name:           baz
Version:        2.0
Release:        1

License:        Public Domain
URL:            None

Summary:        Foo.

Provides:       baz-2.0-provide

%description
Dummy.

%files

%package debuginfo
Summary:        Debug information for foo.
Recommends:     %{name}-debugsource(i686) = %{version}-%{release}

%description debuginfo
Dummy.

%files debuginfo

%package debugsource
Summary:        Debug sources for foo.

%description debugsource
Dummy.

%files debugsource

%changelog
