Feature: Reinstall


Background: Install CQRlib-devel and CQRlib
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install CQRlib-devel"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
        | install-dep   | CQRlib-0:1.1.2-16.fc29.x86_64             |


Scenario: Reinstall an RPM from the same repository
   When I execute dnf with args "reinstall CQRlib"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | reinstall     | CQRlib-0:1.1.2-16.fc29.x86_64             |
    And package state is
        | package                           | reason     | from_repo             |
        | CQRlib-devel-1.1.2-16.fc29.x86_64 | User       | dnf-ci-fedora         |
        | CQRlib-1.1.2-16.fc29.x86_64       | Dependency | dnf-ci-fedora-updates |
    And dnf5 transaction items for transaction "last" are
        | action    | package                       | reason     | repository            |
        | Reinstall | CQRlib-0:1.1.2-16.fc29.x86_64 | Dependency | dnf-ci-fedora-updates |
        | Replaced  | CQRlib-0:1.1.2-16.fc29.x86_64 | Dependency | @System               |


Scenario: Reinstall an RPM from different repository
  Given I use repository "dnf-ci-fedora-updates-testing"
    And I drop repository "dnf-ci-fedora-updates"
   When I execute dnf with args "reinstall CQRlib"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | reinstall     | CQRlib-0:1.1.2-16.fc29.x86_64             |
    And package state is
        | package                           | reason     | from_repo                     |
        | CQRlib-devel-1.1.2-16.fc29.x86_64 | User       | dnf-ci-fedora                 |
        | CQRlib-1.1.2-16.fc29.x86_64       | Dependency | dnf-ci-fedora-updates-testing |
    And dnf5 transaction items for transaction "last" are
        | action    | package                       | reason     | repository                    |
        | Reinstall | CQRlib-0:1.1.2-16.fc29.x86_64 | Dependency | dnf-ci-fedora-updates-testing |
        | Replaced  | CQRlib-0:1.1.2-16.fc29.x86_64 | Dependency | @System                       |


Scenario: Reinstall an RPM that is not available
  Given I drop repository "dnf-ci-fedora-updates"
   When I execute dnf with args "reinstall CQRlib"
   Then the exit code is 1


Scenario: Reinstall list of packages, one of them is not available
   When I execute dnf with args "reinstall CQRlib nosuchpkg"
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


Scenario: Reinstall list of packages with --skip-unavailable, one of them is not available
   When I execute dnf with args "reinstall --skip-unavailable CQRlib nosuchpkg"
   Then the exit code is 0
    And stderr contains lines
        """
        No match for argument: nosuchpkg
        """
    And Transaction is following
        | Action        | Package                                   |
        | reinstall     | CQRlib-0:1.1.2-16.fc29.x86_64             |


Scenario: Reinstall list of packages, one of them is not installed
   When I execute dnf with args "reinstall abcde CQRlib"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Packages for argument 'abcde' available, but not installed.
        """
    And Transaction is empty


Scenario: Reinstall list of packages with --skip-unavailable, one of them is not installed
   When I execute dnf with args "reinstall --skip-unavailable abcde CQRlib"
   Then the exit code is 0
    And stderr contains lines
        """
        Packages for argument 'abcde' available, but not installed.
        """
    And Transaction is following
        | Action        | Package                                   |
        | reinstall     | CQRlib-0:1.1.2-16.fc29.x86_64             |


Scenario: Try to reinstall a pkg if repo not available
  Given I use repository "simple-base"
    And I successfully execute dnf with args "install labirinto"
   When I use repository "simple-base" with configuration
        | key     | value                               |
        | baseurl | https://www.not-available-repo.com/ |
   When I execute dnf with args "reinstall labirinto"
   Then the exit code is 1
   And stderr contains "Failed to download metadata \(baseurl: \"https://www.not-available-repo.com/\"\) for repository \"simple-base\""


Scenario: Try to reinstall a pkg if repo not available
  Given I use repository "simple-base"
    And I successfully execute dnf with args "install labirinto"
   When I configure a new repository "non-existent" with
        | key                 | value                               |
        | baseurl             | https://www.not-available-repo.com/ |
        | enabled             | 1                                   |
        | skip_if_unavailable | 0                                   |
   When I execute dnf with args "reinstall labirinto"
   Then the exit code is 1
   And stderr contains "Failed to download metadata \(baseurl: \"https://www.not-available-repo.com/\"\) for repository \"non-existent\""
