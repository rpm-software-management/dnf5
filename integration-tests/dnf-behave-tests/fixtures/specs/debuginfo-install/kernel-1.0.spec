%undefine _debuginfo_subpackages

Name:           kernel
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        Foo.

Provides:       installonlypkg(kernel)

%description
Dummy.

%files

%package debuginfo
Summary:        Debug information for kernel.
Recommends:     %{name}-debugsource(x86-64) = %{version}-%{release}

%description debuginfo
Dummy.

%files debuginfo

%package debugsource
Summary:        Debug sources for kernel.

%description debugsource
Dummy.

%files debugsource

%changelog
