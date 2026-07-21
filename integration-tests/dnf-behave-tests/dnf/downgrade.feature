Feature: Downgrade command


Background:
  Given I use repository "dnf-ci-fedora-updates"


Scenario: Downgrade one RPM
   When I execute dnf with args "install flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | flac-0:1.3.3-3.fc29.x86_64                |
   When I execute dnf with args "downgrade flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | flac-0:1.3.3-2.fc29.x86_64                |
   When I execute dnf with args "downgrade flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | flac-0:1.3.3-1.fc29.x86_64                |
   When I execute dnf with args "downgrade flac"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        The lowest available version of the "flac.x86_64" package is already installed, cannot downgrade it.
        """


Scenario: Downgrade RPM that requires downgrade of dependency
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-26.fc29.x86_64               |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64        |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
   When I execute dnf with args "downgrade glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | glibc-0:2.28-9.fc29.x86_64                |
        | downgrade     | glibc-common-0:2.28-9.fc29.x86_64         |
        | downgrade     | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
    And package state is
        | package                                | reason     | from_repo     |
        | basesystem-11-6.fc29.noarch            | Dependency | dnf-ci-fedora |
        | filesystem-3.9-2.fc29.x86_64           | Dependency | dnf-ci-fedora |
        | glibc-2.28-9.fc29.x86_64               | User       | dnf-ci-fedora |
        | glibc-all-langpacks-2.28-9.fc29.x86_64 | Dependency | dnf-ci-fedora |
        | glibc-common-2.28-9.fc29.x86_64        | Dependency | dnf-ci-fedora |
        | setup-2.12.1-1.fc29.noarch             | Dependency | dnf-ci-fedora |
    And dnf5 transaction items for transaction "last" are
        | action    | package                                   | reason       | repository    |
        | Downgrade | glibc-0:2.28-9.fc29.x86_64                | User         | dnf-ci-fedora |
        | Downgrade | glibc-common-0:2.28-9.fc29.x86_64         | Dependency   | dnf-ci-fedora |
        | Downgrade | glibc-all-langpacks-0:2.28-9.fc29.x86_64  | Dependency   | dnf-ci-fedora |
        | Replaced  | glibc-0:2.28-26.fc29.x86_64               | User         | @System       |
        | Replaced  | glibc-all-langpacks-0:2.28-26.fc29.x86_64 | Dependency   | @System       |
        | Replaced  | glibc-common-0:2.28-26.fc29.x86_64        | Dependency   | @System       |


Scenario: Downgrade a package that was installed via rpm
  Given I use repository "dnf-ci-fedora"
   When I execute rpm with args "-i --nodeps {context.scenario.repos_location}/dnf-ci-fedora-updates/x86_64/flac-1.3.3-3.fc29.x86_64.rpm"
   Then the exit code is 0
   When I execute dnf with args "downgrade flac"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                    |
        | downgrade | flac-0:1.3.3-2.fc29.x86_64 |
   Then package reasons are
        | Package                  | Reason        |
        | flac-1.3.3-2.fc29.x86_64 | External User |
    And package state is
        | package                  | reason        | from_repo             |
        | flac-1.3.3-2.fc29.x86_64 | External User | dnf-ci-fedora-updates |
    And dnf5 transaction items for transaction "last" are
        | action    | package                    | reason        | repository            |
        | Downgrade | flac-0:1.3.3-2.fc29.x86_64 | External User | dnf-ci-fedora-updates |
        | Replaced  | flac-0:1.3.3-3.fc29.x86_64 | External User | @System               |

Scenario: Downgrade list of packages, none of them has a downgrade available
   When I execute dnf with args "install abcde wget"
   Then the exit code is 0
   When I execute dnf with args "downgrade wget abcde"
   Then the exit code is 0
    And stdout contains "Nothing to do."
    And stderr is
        """
        <REPOSYNC>
        The lowest available version of the "wget.x86_64" package is already installed, cannot downgrade it.
        The lowest available version of the "abcde.noarch" package is already installed, cannot downgrade it.
        """
    And Transaction is empty


Scenario: Downgrade list of packages, one of them is not available
   When I execute dnf with args "install flac"
   Then the exit code is 0
   When I execute dnf with args "downgrade flac nosuchpkg"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: nosuchpkg
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """
    And Transaction is empty


Scenario: Downgrade list of packages with --skip-unavailable, one of them is not available
   When I execute dnf with args "install flac"
   Then the exit code is 0
   When I execute dnf with args "downgrade --skip-unavailable flac nosuchpkg"
   Then the exit code is 0
    And stderr contains lines
    """
    No match for argument: nosuchpkg
    """
    And Transaction is following
        | Action    | Package                    |
        | downgrade | flac-0:1.3.3-2.fc29.x86_64 |


Scenario: Downgrade list of packages, one of them is not installed
   When I execute dnf with args "install flac"
   Then the exit code is 0
   When I execute dnf with args "downgrade flac abcde"
   Then the exit code is 1
    And stderr contains lines
    """
    Failed to resolve the transaction:
    Packages for argument 'abcde' available, but not installed.
    """
    And Transaction is empty


Scenario: Downgrade list of packages with --skip-unavailable, one of them is not installed
   When I execute dnf with args "install flac"
   Then the exit code is 0
   When I execute dnf with args "downgrade --skip-unavailable flac abcde"
   Then the exit code is 0
    And stderr contains lines
    """
    Packages for argument 'abcde' available, but not installed.
    """
    And Transaction is following
        | Action    | Package                    |
        | downgrade | flac-0:1.3.3-2.fc29.x86_64 |


Scenario: Downgrade mixture of not available/not installed/not downgradable/downgradable packages
   When I execute dnf with args "install flac wget"
   Then the exit code is 0
   When I execute dnf with args "downgrade nosuchpkg flac wget abcde"
   Then the exit code is 1
    And stderr contains lines
    """
    Failed to resolve the transaction:
    No match for argument: nosuchpkg
    The lowest available version of the "wget.x86_64" package is already installed, cannot downgrade it.
    Packages for argument 'abcde' available, but not installed.
    You can try to add to command line:
      --skip-unavailable to skip unavailable packages
    """
    And Transaction is empty


Scenario: Downgrade mixture of not available/not installed/not downgradable/downgradable packages with --skip-unavailable
   When I execute dnf with args "install flac wget"
   Then the exit code is 0
   When I execute dnf with args "downgrade --skip-unavailable nosuchpkg flac wget abcde"
   Then the exit code is 0
    And stderr contains lines
    """
    No match for argument: nosuchpkg
    The lowest available version of the "wget.x86_64" package is already installed, cannot downgrade it.
    Packages for argument 'abcde' available, but not installed.
    """
    And Transaction is following
        | Action    | Package                    |
        | downgrade | flac-0:1.3.3-2.fc29.x86_64 |


# https://github.com/rpm-software-management/dnf5/issues/524
Scenario Outline: Check <command> exit code - package does not exist
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "<command> non-existent-package"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: non-existent-package
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """

Examples:
    | command   |
    | upgrade   |
    | downgrade |


Scenario Outline: Check <command> exit code - package not installed
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "<command> flac"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Packages for argument 'flac' available, but not installed.
        """

Examples:
    | command   |
    | upgrade   |
    | downgrade |


@bz1759847
Scenario: Check upgrade exit code - package already on the highest version
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "install flac-0:1.3.3-3.fc29.x86_64"
   When I execute dnf with args "upgrade flac"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Nothing to do.
        """


@bz1759847
Scenario: Check downgrade exit code - package already on the lowest version
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "install flac-0:1.3.2-8.fc29.x86_64"
   When I execute dnf with args "downgrade flac"
   Then the exit code is 0
    And stdout is
        """
        Nothing to do.
        """
    And stderr is
        """
        <REPOSYNC>
        The lowest available version of the "flac.x86_64" package is already installed, cannot downgrade it.
        """
