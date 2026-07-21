Feature: Obsoleted packages

# dnf-ci-obsoletes repo contains:
# PackageA in versions 1.0 and 3.0
# PackageA-Obsoleter, which provides PackageA in version 2.0 and obsoletes PackageA < 2.0
# PackageA-Provider which provides PackageA in versin 4.0

Background: Use dnf-ci-obsoletes repository
  Given I use repository "dnf-ci-obsoletes"


# PackageE has a split in its upgrade-path, PackageA-Obsoleter-1.0-1 obsoletes
# non-best version of PackageE < 2
@bz1902279
Scenario: Install obsoleted package, even though obsoleter of older version is present
   When I execute dnf with args "install PackageE"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageE-0:3.0-1.x86_64                   |
    And package state is
        | package                         | reason | from_repo        |
        | PackageE-3.0-1.x86_64           | User   | dnf-ci-obsoletes |
    And dnf5 transaction items for transaction "last" are
        | action  | package                           | reason | repository       |
        | Install | PackageE-0:3.0-1.x86_64           | User   | dnf-ci-obsoletes |


Scenario: Install alphabetically first of obsoleters when installing obsoleted package
   When I execute dnf with args "install PackageF"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageF-Obsoleter-0:3.0-1.x86_64         |


Scenario: Upgrade a package with multiple obsoleters will install all of them
  Given I execute dnf with args "install PackageF-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageF-0:1.0-1.x86_64                   |
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageF-Obsoleter-0:3.0-1.x86_64         |
        | install       | PackageF-Obsoleter-Second-0:3.0-1.x86_64  |
        | obsoleted     | PackageF-0:1.0-1.x86_64                   |


Scenario: Do not install of obsoleting package using upgrade command, when obsoleted package not on the system
   When I execute dnf with args "upgrade PackageA-Obsoleter"
   Then the exit code is 1
    And Transaction is empty


@bz1818118
Scenario: Install of obsoleting package using upgrade command, when obsoleted package on the system
  Given I execute dnf with args "install PackageE-0:1.0-1.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageE-0:1.0-1.x86_64                   |
   When I execute dnf with args "upgrade PackageA-Obsoleter"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageA-Obsoleter-0:1.0-1.x86_64         |
        | obsoleted     | PackageE-0:1.0-1.x86_64                   |
    And package state is
        | package                         | reason | from_repo        |
        | PackageA-Obsoleter-1.0-1.x86_64 | User   | dnf-ci-obsoletes |
    And dnf5 transaction items for transaction "last" are
        | action   | package                           | reason | repository       |
        | Install  | PackageA-Obsoleter-0:1.0-1.x86_64 | User   | dnf-ci-obsoletes |
        | Replaced | PackageE-0:1.0-1.x86_64           | User   | @System          |


@xfail
# https://github.com/rpm-software-management/dnf5/issues/1783
Scenario: Obsoleting a package that was installed via rpm, with --best
   When I execute rpm with args "-i --nodeps {context.scenario.repos_location}/dnf-ci-obsoletes/x86_64/PackageB-1.0-1.x86_64.rpm"
   Then the exit code is 0
   When I execute dnf with args "upgrade --best"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-Obsoleter-0:1.0-1.x86_64         |
        | obsoleted     | PackageB-0:1.0-1.x86_64                   |
    And package state is
        | package                         | reason          | from_repo        |
        | PackageB-Obsoleter-1.0-1.x86_64 | External User   | dnf-ci-obsoletes |
    And dnf5 transaction items for transaction "last" are
        | action   | package                           | reason        | repository       |
        | Install  | PackageB-Obsoleter-0:1.0-1.x86_64 | External User | dnf-ci-obsoletes |
        | Replaced | PackageB-0:1.0-1.x86_64           | External User | @System          |


Scenario: Obsoleting a package that was installed via rpm, with --nobest
   When I execute rpm with args "-i --nodeps {context.scenario.repos_location}/dnf-ci-obsoletes/x86_64/PackageB-1.0-1.x86_64.rpm"
   Then the exit code is 0
   When I execute dnf with args "upgrade --nobest"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-Obsoleter-0:1.0-1.x86_64         |
        | obsoleted     | PackageB-0:1.0-1.x86_64                   |
    And package state is
        | package                         | reason        | from_repo        |
        | PackageB-Obsoleter-1.0-1.x86_64 | External User | dnf-ci-obsoletes |
    And dnf5 transaction items for transaction "last" are
        | action   | package                           | reason        | repository       |
        | Install  | PackageB-Obsoleter-0:1.0-1.x86_64 | External User | dnf-ci-obsoletes |
        | Replaced | PackageB-0:1.0-1.x86_64           | External User | @System          |


@bz1818118
Scenario: Install of obsoleting package from commandline using upgrade command, when obsoleted package on the system
  Given I execute dnf with args "install PackageE-0:1.0-1.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageE-0:1.0-1.x86_64                   |
   When I execute dnf with args "upgrade {context.dnf.fixturesdir}/repos/dnf-ci-obsoletes/x86_64/PackageA-Obsoleter-1.0-1.x86_64.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageA-Obsoleter-0:1.0-1.x86_64         |
        | obsoleted     | PackageE-0:1.0-1.x86_64                   |
    And package state is
        | package                         | reason | from_repo    |
        | PackageA-Obsoleter-1.0-1.x86_64 | User   | @commandline |
    And dnf5 transaction items for transaction "last" are
        | action   | package                           | reason | repository   |
        | Install  | PackageA-Obsoleter-0:1.0-1.x86_64 | User   | @commandline |
        | Replaced | PackageE-0:1.0-1.x86_64           | User   | @System      |


Scenario: Upgrade of obsoleted package by package of higher version than obsoleted
   When I execute dnf with args "install PackageA-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageA-0:1.0-1.x86_64                   |
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | PackageA-0:3.0-1.x86_64                   |
    And package state is
        | package               | reason | from_repo        |
        | PackageA-3.0-1.x86_64 | User   | dnf-ci-obsoletes |
    And dnf5 transaction items for transaction "last" are
        | action   | package                 | reason | repository       |
        | Upgrade  | PackageA-0:3.0-1.x86_64 | User   | dnf-ci-obsoletes |
        | Replaced | PackageA-0:1.0-1.x86_64 | User   | @System          |


Scenario: Install of obsoleted package
   When I execute dnf with args "install PackageB"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-Obsoleter-0:1.0-1.x86_64         |
    And package state is
        | package                         | reason | from_repo        |
        | PackageB-Obsoleter-1.0-1.x86_64 | User   | dnf-ci-obsoletes |
    And dnf5 transaction items for transaction "last" are
        | action  | package                           | reason | repository       |
        | Install | PackageB-Obsoleter-0:1.0-1.x86_64 | User   | dnf-ci-obsoletes |


Scenario: Upgrade of obsoleted package
   When I execute dnf with args "install PackageB-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-0:1.0-1.x86_64                   |
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-Obsoleter-0:1.0-1.x86_64         |
        | obsoleted     | PackageB-0:1.0-1.x86_64                   |
    And package state is
        | package                         | reason | from_repo        |
        | PackageB-Obsoleter-1.0-1.x86_64 | User   | dnf-ci-obsoletes |
    And dnf5 transaction items for transaction "last" are
        | action   | package                           | reason | repository       |
        | Install  | PackageB-Obsoleter-0:1.0-1.x86_64 | User   | dnf-ci-obsoletes |
        | Replaced | PackageB-0:1.0-1.x86_64           | User   | @System          |


Scenario: Upgrade of obsoleted package if package specified by version with glob (no obsoletes applied)
   When I execute dnf with args "install PackageB-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-0:1.0-1.x86_64                   |
   When I execute dnf with args "upgrade PackageB-2*"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | PackageB-0:2.0-1.x86_64                   |
    And package state is
        | package               | reason | from_repo        |
        | PackageB-2.0-1.x86_64 | User   | dnf-ci-obsoletes |
    And dnf5 transaction items for transaction "last" are
        | action   | package                 | reason | repository       |
        | Upgrade  | PackageB-0:2.0-1.x86_64 | User   | dnf-ci-obsoletes |
        | Replaced | PackageB-0:1.0-1.x86_64 | User   | @System          |


@xfail
# https://github.com/rpm-software-management/dnf5/issues/1783
@bz1672618
Scenario: Keep reason of obsoleted package
   When I execute dnf with args "install PackageB-1.0"
   Then the exit code is 0
   When I execute dnf with args "mark dependency PackageB"
   Then the exit code is 0
    And package state is
        | package               | reason       | from_repo        |
        | PackageB-1.0-1.x86_64 | Dependency   | dnf-ci-obsoletes |
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-Obsoleter-0:1.0-1.x86_64         |
        | obsoleted     | PackageB-0:1.0-1.x86_64                   |
    And package state is
        | package                          | reason       | from_repo        |
        | PackageB-Obsoleter-1.0-1.x86_64  | Dependency   | dnf-ci-obsoletes |


Scenario: Autoremoval of obsoleted package
   When I execute dnf with args "install PackageB-1.0"
   Then the exit code is 0
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | PackageB-Obsoleter-0:1.0-1.x86_64         |
        | obsoleted     | PackageB-0:1.0-1.x86_64                   |
    And package state is
        | package                         | reason | from_repo        |
        | PackageB-Obsoleter-1.0-1.x86_64 | User   | dnf-ci-obsoletes |
    And dnf5 transaction items for transaction "last" are
        | action   | package                           | reason | repository       |
        | Install  | PackageB-Obsoleter-0:1.0-1.x86_64 | User   | dnf-ci-obsoletes |
        | Replaced | PackageB-0:1.0-1.x86_64           | User   | @System          |
   When I execute dnf with args "autoremove"
   Then the exit code is 0
    But Transaction is empty


@bz1672947
Scenario: Multilib obsoletes during distro-sync
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install lz4-0:1.7.5-2.fc26.i686 lz4-0:1.7.5-2.fc26.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | lz4-0:1.7.5-2.fc26.i686       |
        | install       | lz4-0:1.7.5-2.fc26.x86_64     |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "distro-sync"
   Then the exit code is 0
   Then stderr does not contain "TransactionItem not found for key: lz4"
    And Transaction is following
        | Action        | Package                               |
        | upgrade       | lz4-0:1.8.2-2.fc29.x86_64             |
        | upgrade       | lz4-0:1.8.2-2.fc29.i686               |
        | install       | lz4-libs-0:1.8.2-2.fc29.x86_64        |
    And dnf5 transaction items for transaction "last" are
        | action   | package                           | reason | repository            |
        | Install  | lz4-libs-0:1.8.2-2.fc29.x86_64    | User   | dnf-ci-fedora-updates |
        | Upgrade  | lz4-0:1.8.2-2.fc29.x86_64         | User   | dnf-ci-fedora-updates |
        | Upgrade  | lz4-0:1.8.2-2.fc29.i686           | User   | dnf-ci-fedora-updates |
        | Replaced | lz4-0:1.7.5-2.fc26.x86_64         | User   | @System               |
        | Replaced | lz4-0:1.7.5-2.fc26.i686           | User   | @System               |


# PackageD-0:2.0-1.x86_64 obsoletes PackageC < 2
# PackageD-0:1.0-1.x86_64 does not obsolete anything
@bz1761137
Scenario: Obsoleted package is not installed when group contains both obsoleter and obsoleted packages
   When I execute dnf with args "group install obsoleter-obsoleted"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install-group | PackageD-0:2.0-1.x86_64               |
        | group-install | Obsoleter and obsoleted               |


@bz1761137
Scenario: Both packages are installed when group contains both obsoleter and obsoleted packages and obsoletes are switched off
   When I execute dnf with args "group install obsoleter-obsoleted --setopt=obsoletes=False"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Problem: package PackageD-2.0-1.x86_64 from dnf-ci-obsoletes obsoletes PackageC < 2.0 provided by PackageC-1.0-1.x86_64 from dnf-ci-obsoletes
          - cannot install the best candidate for the job
          - conflicting requests
        You can try to add to command line:
          --no-best to not limit the transaction to the best candidates
          --skip-broken to skip uninstallable packages
        """
