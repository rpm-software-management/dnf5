Feature: distro-sync


@dnf5daemon
Scenario: when there is noting to do
Given I use repository "simple-base"
 When I execute dnf with args "distro-sync"
 Then the exit code is 0
  And Transaction is empty


@dnf5daemon
Scenario: updating a pkg
Given I use repository "simple-base"
  And I execute dnf with args "install labirinto"
  And I use repository "simple-updates"
 When I execute dnf with args "distro-sync"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                               |
      | upgrade       | labirinto-2.0-1.fc29.x86_64           |

@dnf5daemon
Scenario: Ignore excluded packages - not fail on excluded best candidate
Given I use repository "simple-base"
  And I execute dnf with args "install labirinto"
  And I use repository "simple-updates"
 When I execute dnf with args "distro-sync --setopt=excludepkgs=labirinto-2.0-1.fc29.x86_64 --setopt=best=True"
 Then the exit code is 0
  And Transaction is empty


@dnf5daemon
Scenario: updating a signed pkg
Given I use repository "simple-base"
  And I successfully execute dnf with args "install dedalo"
  And I use repository "simple-updates"
 When I execute dnf with args "distro-sync"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                        |
      | upgrade       | dedalo-2.0-1.fc29.x86_64       |


Scenario: updating a signed pkg without key specified
Given I use repository "dnf-ci-fedora"
  And I successfully execute dnf with args "install wget"
  And I use repository "dnf-ci-gpg-updates" with configuration
      | key      | value      |
      | gpgcheck | 1          |
 When I execute dnf with args "distro-sync"
 Then the exit code is 1
  And stderr contains "Transaction failed: Signature verification failed."


Scenario: updating a broken signed pkg whose key is not imported
Given I use repository "dnf-ci-gpg"
  And I execute dnf with args "install wget"
  And I use repository "dnf-ci-gpg-updates" with configuration
      | key      | value      |
      | gpgcheck | 1          |
      | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-updates/dnf-ci-gpg-updates-public |
 When I execute dnf with args "distro-sync wget"
 Then the exit code is 1
  And stderr contains lines matching
    """
    Transaction failed: Signature verification failed\.
    OpenPGP check for package "wget-2\.0\.0-1\.fc29\.x86_64" \(.*/wget-2.0.0-1.fc29.x86_64.rpm\) from repo "dnf-ci-gpg-updates" has failed: Problem occurred when opening the package\.
    """


@bz1963732
@not.with_os=rhel__ge__8
Scenario: updating a broken signed pkg whose key is imported
Given I use repository "dnf-ci-gpg"
  And I execute dnf with args "install wget"
  And I use repository "dnf-ci-gpg-updates" with configuration
      | key      | value      |
      | gpgcheck | 1          |
      | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-updates/dnf-ci-gpg-updates-public |
  And I successfully execute rpm with args "--import {context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-updates/dnf-ci-gpg-updates-public"
 When I execute dnf with args "distro-sync wget"
 Then the exit code is 1
  And stderr contains lines matching
    """
    Transaction failed: Signature verification failed\.
    OpenPGP check for package "wget-2\.0\.0-1\.fc29\.x86_64" \(.*/wget-2.0.0-1.fc29.x86_64.rpm\) from repo "dnf-ci-gpg-updates" has failed: Problem occurred when opening the package\.
    """


Scenario: distro-sync list of packages, one of them is not available
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "distro-sync flac nosuchpkg"
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


Scenario: distro-sync list of packages with --skip-unavailable, one of them is not available
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "distro-sync --skip-unavailable flac nosuchpkg"
   Then the exit code is 0
    And stderr contains lines
    """
    No match for argument: nosuchpkg
    """
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |


Scenario: distro-sync list of packages, one of them is not installed
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "distro-sync flac dwm"
   Then the exit code is 1
    And stderr contains lines
    """
    Failed to resolve the transaction:
    Packages for argument 'dwm' available, but not installed.
    """
    And Transaction is empty


Scenario: distro-sync list of packages with --skip-unavailable, one of them is not installed
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "distro-sync --skip-unavailable dwm flac"
   Then the exit code is 0
    And stderr contains lines
    """
    Packages for argument 'dwm' available, but not installed.
    """
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |


Scenario: distro-sync all with a broken dependency and without best
  Given I use repository "upgrade-dependent"
    And I successfully execute dnf with args "install labirinto"
    And I drop repository "upgrade-dependent"
    And I use repository "broken-distrosync"
   When I execute dnf with args "distro-sync --no-best"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Problem: installed package labirinto-2.0-1.noarch requires labirinto-libs = 2.0-1, but none of the providers can be installed
          - labirinto-libs-2.0-1.noarch does not belong to a distupgrade repository
          - problem with installed package
        You can try to add to command line:
          --skip-broken to skip uninstallable packages
        """
    And stdout is empty
