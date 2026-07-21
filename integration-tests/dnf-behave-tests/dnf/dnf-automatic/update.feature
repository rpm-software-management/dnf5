Feature: dnf-automatic performs update


Scenario: dnf-automatic can update package
  Given I use repository "simple-base"
    And I successfully execute dnf with args "install labirinto"
    And I use repository "simple-updates"
   When I execute dnf with args "automatic --installupdates"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | upgrade       | labirinto-0:2.0-1.fc29.x86_64         |


@bz1793298
Scenario: dnf-automatic fails to update when the update package is not signed
  Given I use repository "unsigned"
    And I successfully execute dnf with args "install --nogpgcheck sarcina-1.0"
   When I execute dnf with args "automatic --installupdates"
   Then the exit code is 1
    And RPMDB Transaction is empty
    And stdout contains "OpenPGP check for package \"sarcina-2.0-1.fc29.x86_64\" \(.*\) from repo \"unsigned\" has failed: The package is not signed."


@bz1793298
Scenario: dnf-automatic fails to update when the public gpg key is not installed
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "install wget"
    And I use repository "dnf-ci-gpg-updates" with configuration
        | key      | value  |
        | gpgcheck | 1      |
   When I execute dnf with args "automatic --installupdates"
   Then the exit code is 1
    And stdout contains "OpenPGP check for package \"wget-2.0.0-1.fc29.x86_64\" \(.*\) from repo \"dnf-ci-gpg-updates\" has failed: Problem occurred when opening the package."
    And stderr is empty
    And RPMDB Transaction is empty
