Name:           with-build-conflict
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        A dummy package.

# package `build-requirement-a` recommends `weak-dependency`
BuildRequires:  build-requirement-a
BuildConflicts: weak-dependency

%description
Dummy.

%files

%changelog
