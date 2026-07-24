Feature: Tests locking capabilities of the versionlock plugin


Background: Set up versionlock infrastructure in the installroot
  Given I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "wget"
    [[packages.conditions]]
    key = "evr"
    comparator = "="
    value = "0:1.19.5-5.fc29"
    """
  # check that both locked and newer versions of the package are available
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "repoquery wget"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    wget-0:1.19.5-5.fc29.src
    wget-0:1.19.5-5.fc29.x86_64
    wget-0:1.19.6-5.fc29.src
    wget-0:1.19.6-5.fc29.x86_64
    """


Scenario: I can list all versions of locked package
   When I execute dnf with args "list --showduplicates wget"
   Then stderr is
    """
    <REPOSYNC>
    """
   And stdout matches line by line
    """
    Available packages
    wget.src +1.19.5-5.fc29 +dnf-ci-fedora
    wget.x86_64 +1.19.5-5.fc29 +dnf-ci-fedora
    wget.src +1.19.6-5.fc29 +dnf-ci-fedora-updates
    wget.x86_64 +1.19.6-5.fc29 +dnf-ci-fedora-updates
    """


Scenario: The locked version of the package gets installed although newer one is available
   When I execute dnf with args "install wget"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | wget-0:1.19.5-5.fc29.x86_64           |


Scenario: The locked version of the package gets installed as a dependency
   When I execute dnf with args "install abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | abcde-0:2.9.3-1.fc29.noarch           |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64           |
        | install-weak  | flac-0:1.3.3-3.fc29.x86_64            |


Scenario: I can reinstall the installed locked version of the package
  Given I successfully execute dnf with args "install wget-1.19.5"
   When I execute dnf with args "reinstall wget"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | wget-0:1.19.5-5.fc29.x86_64           |


Scenario: I cannot upgrade the locked version of the package
  Given I successfully execute dnf with args "install wget-1.19.5"
   When I execute dnf with args "upgrade wget"
   Then the exit code is 0
    And Transaction is empty


Scenario: I can remove the installed locked version of the package
  Given I successfully execute dnf with args "install wget-1.19.5"
   When I execute dnf with args "remove wget"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | wget-0:1.19.5-5.fc29.x86_64           |


@bz1780370
Scenario: I can remove installed package when other version is locked
  Given I successfully execute dnf with args "install wget-1.19.5"
    And I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "wget"
    [[packages.conditions]]
    key = "evr"
    comparator = "="
    value = "0:2.0.0-1.fc29"
    """
   When I execute dnf with args "remove wget"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | wget-0:1.19.5-5.fc29.x86_64           |


@bz1431491
Scenario: Locking does not require that the package exists in a repository
  Given I drop repository "dnf-ci-fedora"
   When I execute dnf with args "install wget"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Argument 'wget' matches only packages excluded by versionlock.
        """


@bz1726712
Scenario: I can upgrade to the locked version of the package when older version is installed
  Given I successfully execute dnf with args "install wget-1.19.5"
    And I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "wget"
    [[packages.conditions]]
    key = "evr"
    comparator = "="
    value = "0:1.19.6-5.fc29"
    """
   When I execute dnf with args "upgrade wget"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64           |


Scenario: Versionlock can lock the package version to an interval
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
    And I use repository "dnf-ci-fedora-updates-testing"
    And I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "flac"
    [[packages.conditions]]
    key = "evr"
    comparator = ">="
    value = "1.3"
    [[packages.conditions]]
    key = "evr"
    comparator = "<"
    value = "1.4"
    """
   When I execute dnf with args "repoquery flac"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    flac-0:1.3.2-8.fc29.src
    flac-0:1.3.2-8.fc29.x86_64
    flac-0:1.3.3-1.fc29.src
    flac-0:1.3.3-1.fc29.x86_64
    flac-0:1.3.3-2.fc29.src
    flac-0:1.3.3-2.fc29.x86_64
    flac-0:1.3.3-3.fc29.src
    flac-0:1.3.3-3.fc29.x86_64
    flac-0:1.4.0-1.fc29.src
    flac-0:1.4.0-1.fc29.x86_64
    """
   When I execute dnf with args "install flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | flac-0:1.3.3-3.fc29.x86_64            |


@bz1750620
Scenario: Check-update command does not report updates filtered out by the versionlock
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac-0:1.3.2-8.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | flac-0:1.3.2-8.fc29.x86_64            |
  Given I use repository "dnf-ci-fedora-updates"
    And I use repository "dnf-ci-fedora-updates-testing"
  # no versionlock rule for the flac package
  Given I create file "/etc/dnf/versionlock.toml" with
    """
    """
   When I execute dnf with args "check-upgrade"
   Then the exit code is 100
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Upgrades
    flac.x86_64 1.4.0-1.fc29 dnf-ci-fedora-updates-testing
    """
  # flac package versionlocked on specific minor version
  Given I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "flac"
    [[packages.conditions]]
    key = "evr"
    comparator = ">="
    value = "1.3"
    [[packages.conditions]]
    key = "evr"
    comparator = "<"
    value = "1.4"
    """
   When I execute dnf with args "check-upgrade"
   Then the exit code is 100
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Upgrades
    flac.x86_64 1.3.3-3.fc29 dnf-ci-fedora-updates
    """
  # flac package versionlocked on specific version
  Given I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "flac"
    [[packages.conditions]]
    key = "evr"
    comparator = "="
    value = "1.3.2-8.fc29"
    """
   When I execute dnf with args "check-upgrade"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is empty


# PackageB-Obsoleter obsoletes PackageB < 3.0
@bz1627124
Scenario: The locked version of the package cannot get obsoleted
  Given I use repository "dnf-ci-obsoletes"
    And I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "PackageB"
    [[packages.conditions]]
    key = "evr"
    comparator = "="
    value = "1.0-1"
    """
   When I execute dnf with args "install PackageB-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-0:1.0-1.x86_64                   |
   When I execute dnf with args "upgrade PackageB"
   Then the exit code is 0
    And Transaction is empty


# PackageD-2.0 obsoletes PackageC < 2.0
@bz1627124
Scenario: The locked version of the package cannot get obsoleted by upgrade of other package
  Given I use repository "dnf-ci-obsoletes"
    And I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "PackageC"
    [[packages.conditions]]
    key = "evr"
    comparator = "="
    value = "1.0-1"
    """
   When I execute dnf with args "install PackageC-1.0 PackageD-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageC-0:1.0-1.x86_64                   |
        | install       | PackageD-0:1.0-1.x86_64                   |
   When I execute dnf with args "upgrade PackageD"
   Then the exit code is 0
    And Transaction is empty


@bz1957280
Scenario: When both obsoleted and obsoleter are locked, the obsoleter package is not filtered out and can be installed
  Given I use repository "dnf-ci-obsoletes"
    And I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "PackageB"
    [[packages.conditions]]
    key = "evr"
    comparator = "="
    value = "1.0-1"
    [[packages]]
    name = "PackageB-Obsoleter"
    [[packages.conditions]]
    key = "evr"
    comparator = "="
    value = "1.0-1"
    """
   When I execute dnf with args "install PackageB-Obsoleter"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-Obsoleter-0:1.0-1.x86_64         |


@bz1961217
Scenario: The packages with minorbump part of release are correctly locked
  Given I use repository "miscellaneous"
    And I successfully execute dnf with args "install minorbump-0:1.0-1.fc29.x86_64"
    And I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [[packages]]
    name = "minorbump"
    [[packages.conditions]]
    key = "evr"
    comparator = "="
    value = "0:1.0-1.fc29"
    """
        # check that minorbump is available with version higher then locked
    And I successfully execute dnf with args "repoquery minorbump"
   Then stderr is
    """
    <REPOSYNC>
    """
   And stdout is
    """
    minorbump-0:1.0-1.fc29.1.src
    minorbump-0:1.0-1.fc29.1.x86_64
    minorbump-0:1.0-1.fc29.src
    minorbump-0:1.0-1.fc29.x86_64
    """
   When I execute dnf with args "upgrade minorbump"
   Then the exit code is 0
    And Transaction is empty
