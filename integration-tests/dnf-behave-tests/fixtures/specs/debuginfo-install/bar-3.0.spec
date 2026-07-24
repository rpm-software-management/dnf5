%undefine _debuginfo_subpackages

Name:           bar
Version:        3.0
Release:        1

License:        Public Domain
URL:            None

Summary:        Foo.

%description
Dummy.

%files

%package debuginfo
Summary:        Debug information for foo.
Recommends:     %{name}-debugsource(x86-64) = %{version}-%{release}

%description debuginfo
Dummy.

%files debuginfo

%package debugsource
Summary:        Debug sources for foo.

%description debugsource
Dummy.

%files debugsource

%changelog
