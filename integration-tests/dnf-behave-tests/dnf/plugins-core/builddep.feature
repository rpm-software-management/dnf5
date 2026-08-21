Feature: dnf builddep command


Scenario: Dnf builddep can use spec file from a remote location
  Given I use repository "dnf-ci-fedora"
    And I create directory "/remotedir"
    And I create file "/remotedir/pkg.spec" with
    """
    Name: pkg
    Version: 1
    Release: 1
    Summary: summary
    License: license

    BuildRequires: filesystem

    %description
    desc
    """
    And I set up a http server for directory "/remotedir"
   When I execute dnf with args "builddep http://localhost:{context.dnf.ports[/remotedir]}/pkg.spec"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


Scenario: Builddep with simple dependency (spec)
    Given I use repository "dnf-ci-fedora"
     When I execute dnf with args "builddep {context.dnf.fixturesdir}/specs/dnf-ci-thirdparty/SuperRipper-1.0-1.spec"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | lame-libs-0:3.100-4.fc29.x86_64   |


Scenario: Builddep with simple dependency (spec) + define
    Given I use repository "dnf-ci-fedora"
     When I execute dnf with args "builddep {context.dnf.fixturesdir}/specs/dnf-ci-thirdparty/SuperRipper-1.0-1.spec --define 'buildrequires flac'"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | flac-0:1.3.2-8.fc29.x86_64        |


Scenario: Builddep with simple dependency (srpm)
    Given I use repository "dnf-ci-fedora"
     When I execute dnf with args "builddep {context.dnf.fixturesdir}/repos/dnf-ci-thirdparty/src/SuperRipper-1.0-1.src.rpm"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | lame-libs-0:3.100-4.fc29.x86_64   |


@not.with_os=rhel__eq__7
Scenario: Builddep with rich dependency
    Given I use repository "dnf-ci-fedora"
     When I execute dnf with args "builddep {context.dnf.fixturesdir}/specs/dnf-ci-thirdparty/SuperRipper-1.0-1.spec --define 'buildrequires (flac and lame-libs)'"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | flac-0:1.3.2-8.fc29.x86_64        |
        | install       | lame-libs-0:3.100-4.fc29.x86_64   |


Scenario: Builddep with simple dependency (files-like provide)
    Given I use repository "dnf-ci-fedora"
     When I execute dnf with args "builddep {context.dnf.fixturesdir}/specs/dnf-ci-thirdparty/SuperRipper-1.0-1.spec --define 'buildrequires /etc/ld.so.conf'"
     Then the exit code is 0
      And Transaction contains
        | Action        | Package                           |
        | install       | glibc-0:2.28-9.fc29.x86_64        |


Scenario: Builddep with simple dependency (non-existent)
    Given I use repository "dnf-ci-fedora"
     When I execute dnf with args "builddep {context.dnf.fixturesdir}/specs/dnf-ci-thirdparty/SuperRipper-1.0-1.spec --define 'buildrequires flac = 15'"
     Then the exit code is 1
      And stderr is
          """
          <REPOSYNC>
          Failed to resolve the transaction:
          No match for argument: flac = 15
          You can try to add to command line:
            --skip-unavailable to skip unavailable packages
          """


@bz1724668
Scenario: Builddep on SPEC with non-available Source0
 Given I create file "{context.dnf.installroot}/missingSource.spec" with
   """
   Name: dummy-pkg
   Summary: dummy-pkg summary
   Version: 1.0
   Release: 1
   License: GPL
   Source0: no-such-archive.tar.gz
   %description
   This is a dummy-pkg description
   %build
   %files
   %changelog
   """
  When I execute dnf with args "builddep {context.dnf.installroot}/missingSource.spec"
  Then the exit code is 1
   And stderr matches line by line
       """
       <REPOSYNC>
       error: Unable to open .*/missingSource.spec: No such file or directory
       Failed to parse spec file ".*/missingSource.spec".
       Failed to parse some inputs.
       """


@bz1758459
Scenario: I exclude the highest verion of a package and call dnf builddep with --best
  Given I use repository "dnf-ci-fedora-updates"
    And I create file "dummy-pkg.spec" with
   """
   Name: dummy-pkg
   Summary: dummy-pkg summary
   Version: 1.0
   Release: 1
   License: GPL
   BuildRequires: flac
   %description
   This is a dummy-pkg description
   %build
   %files
   %changelog
   """
   When I execute dnf with args "builddep {context.dnf.installroot}/dummy-pkg.spec --exclude flac-1.3.3-3.fc29 --best"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                    |
        | install               | flac-0:1.3.3-2.fc29.x86_64 |


@bz1628634
Scenario: Builddep with unavailable build dependency
    Given I use repository "dnf-ci-fedora"
     When I execute dnf with args "builddep {context.dnf.fixturesdir}/repos/dnf-ci-builddep/src/unavailable-requirement-1.0-1.src.rpm"
     Then the exit code is 1
      And stderr is
          """
          <REPOSYNC>
          Failed to resolve the transaction:
          No match for argument: this-lib-is-not-available
          You can try to add to command line:
            --skip-unavailable to skip unavailable packages
          """
     When I execute dnf with args "builddep --skip-unavailable {context.dnf.fixturesdir}/repos/dnf-ci-builddep/src/unavailable-requirement-1.0-1.src.rpm"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | lame-libs-0:3.100-4.fc29.x86_64   |


@bz2077820
Scenario: Builddep using macros with source rpm
    Given I use repository "dnf-ci-fedora"
     When I execute dnf with args "builddep -D 'dummy_param 1' {context.dnf.fixturesdir}/repos/dnf-ci-thirdparty/src/SuperRipper-1.0-1.src.rpm"
     Then the exit code is 0
      And stderr contains lines
      """
      Warning: -D/--define/--with/--without arguments have no effect on source rpm packages.
      """


Scenario: Builddep where package BuildRequires a pkg spec that contains glob characters
    Given I use repository "builddep"
     When I execute dnf with args "builddep requires-glob"
     Then the exit code is 0
      And Transaction is following
          | Action        | Package                    |
          | install       | provides-glob-0:1-1.x86_64 |


Scenario: Builddep respects --with option
  Given I use repository "dnf-ci-fedora"
    And I create file "/pkg.spec" with
    """
    # disabled foo
    %bcond_with  foo

    Name: pkg
    Version: 1
    Release: 1
    Summary: summary
    License: license

    %if %{with foo}
    BuildRequires: filesystem
    %endif

    %description
    desc
    """
   When I execute dnf with args "builddep {context.dnf.installroot}/pkg.spec"
   Then the exit code is 0
    And Transaction is empty
   When I execute dnf with args "builddep {context.dnf.installroot}/pkg.spec --with foo"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                        |
        | install       | filesystem-0:3.9-2.fc29.x86_64 |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch   |


Scenario: Builddep respects --without option
  Given I use repository "dnf-ci-fedora"
    And I create file "/pkg.spec" with
    """
    # enabled foo
    %bcond_without  foo

    Name: pkg
    Version: 1
    Release: 1
    Summary: summary
    License: license

    %if %{without foo}
    BuildRequires: filesystem
    %endif

    %description
    desc
    """
   When I execute dnf with args "builddep {context.dnf.installroot}/pkg.spec"
   Then the exit code is 0
    And Transaction is empty
   When I execute dnf with args "builddep {context.dnf.installroot}/pkg.spec --without foo"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                        |
        | install       | filesystem-0:3.9-2.fc29.x86_64 |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch   |


Scenario: Builddep respects both --with and --without option
  Given I use repository "dnf-ci-fedora"
    And I create file "/pkg.spec" with
    """
    # enabled foo
    %bcond_without  foo
    # disabled bar
    %bcond_with     bar

    Name: pkg
    Version: 1
    Release: 1
    Summary: summary
    License: license

    %if %{without foo}
    BuildRequires: dwm
    %endif

    %if %{with bar}
    BuildRequires: wget
    %endif

    %description
    desc
    """
   When I execute dnf with args "builddep {context.dnf.installroot}/pkg.spec"
   Then the exit code is 0
    And Transaction is empty
   When I execute dnf with args "builddep {context.dnf.installroot}/pkg.spec --without foo --with bar "
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                     |
        | install       | dwm-0:6.1-1.x86_64          |
        | install       | wget-0:1.19.5-5.fc29.x86_64 |


Scenario: User can specify parameter type using --spec option
  Given I use repository "dnf-ci-fedora"
    And I create file "/pkg.spec.in" with
    """
    Name: pkg
    Version: 1
    Release: 1
    Summary: summary
    License: license

    BuildRequires: filesystem

    %description
    desc
    """
   When I execute dnf with args "builddep --spec {context.dnf.installroot}/pkg.spec.in"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


Scenario: User can specify parameter type using --srpm option
    Given I use repository "dnf-ci-fedora"
      And I copy file "{context.dnf.fixturesdir}/repos/dnf-ci-thirdparty/src/SuperRipper-1.0-1.src.rpm" to "/SuperRipper-1.0-1.src.rpm.unknown.extension"
     When I execute dnf with args "builddep --srpm {context.dnf.installroot}/SuperRipper-1.0-1.src.rpm.unknown.extension"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | lame-libs-0:3.100-4.fc29.x86_64   |
