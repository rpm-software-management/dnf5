Feature: Transaction history redo - comps


Background:
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
    And I successfully execute dnf with args "group install dnf-ci-testgroup"
   Then Transaction is following
        | Action        | Package                           |
        | group-install | DNF-CI-Testgroup                  |
        | install-group | filesystem-0:3.9-2.fc29.x86_64    |
        | install-group | lame-0:3.100-4.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64   |
    And History is following
        | Id     | Command                               | Action | Altered   |
        | 1      | group install dnf-ci-testgroup        |        | 5         |


Scenario: Redo a transaction that installed a group
  Given I successfully execute dnf with args "remove lame"
   When I execute dnf with args "history redo 1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | lame-0:3.100-4.fc29.x86_64        |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64   |
    And stderr contains "Group \"dnf-ci-testgroup\" is already installed."
    And History is following
        | Id     | Command                               | Action | Altered   |
        | 3      | history redo 1                        |        | 2         |
        | 2      | remove lame                           |        | 2         |
        | 1      | group install dnf-ci-testgroup        |        | 5         |


Scenario: Redo a transaction that removed a group
  Given I successfully execute dnf with args "group remove dnf-ci-testgroup"
    And I successfully execute dnf with args "install lame"
   When I execute dnf with args "history redo 2"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | lame-0:3.100-4.fc29.x86_64        |
        | remove-unused | lame-libs-0:3.100-4.fc29.x86_64   |
    And History is following
        | Id     | Command                               | Action | Altered   |
        | 4      | history redo 2                        |        | 2         |
        | 3      | install lame                          |        | 2         |
        | 2      | group remove dnf-ci-testgroup         |        | 5         |
        | 1      | group install dnf-ci-testgroup        |        | 5         |


Scenario: Redo a transaction with a missing group
  Given I successfully execute dnf with args "group remove dnf-ci-testgroup"
    And I drop repository "dnf-ci-thirdparty"
   When I execute dnf with args "history redo 1"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: dnf-ci-testgroup
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """


Scenario: Redo a transaction that removed a group and the group is was removed from the system already
  Given I successfully execute dnf with args "group remove dnf-ci-testgroup"
   When I execute dnf with args "history redo last"
   Then the exit code is 0
    And Transaction is empty


Scenario: Redo a transaction that installed a group and the group is still on the system
   When I execute dnf with args "history redo last"
   Then the exit code is 0
    And Transaction is empty
    And stderr contains "Group \"dnf-ci-testgroup\" is already installed."
    And History is following
        | Id     | Command                               | Action | Altered   |
        | 1      | group install dnf-ci-testgroup        |        | 5         |


Scenario: Redo a transaction that upgraded a group and the group is still on the system
  Given I successfully execute dnf with args "group upgrade dnf-ci-testgroup"
   When I execute dnf with args "history redo last"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-upgrade | DNF-CI-Testgroup                  |
    And History is following
        | Id     | Command                               | Action | Altered   |
        | 3      | history redo last                     |        | 1         |
        | 2      | group upgrade dnf-ci-testgroup        |        | 1         |
        | 1      | group install dnf-ci-testgroup        |        | 5         |
