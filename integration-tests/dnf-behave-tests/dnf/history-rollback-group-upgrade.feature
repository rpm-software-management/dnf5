Feature: Transaction history rollback of group upgrades


Background:
  Given I use repository "dnf-ci-group-rollback-1"
    And I successfully execute dnf with args "group install dnf-ci-group-rollback-testgroup1"
   Then Transaction is following
        | Action        | Package                                       |
        | group-install | DNF-CI-RollbackTestGroup1                     |
        | install-group | TestGroup1PackageA-0:1.0-1.x86_64             |
        | install-group | TestGroup1PackageB-0:1.0-1.x86_64             |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
   When I execute dnf with args "group list --installed dnf-ci-group-rollback-testgroup1"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ID                               Name                      Installed
        dnf-ci-group-rollback-testgroup1 DNF-CI-RollbackTestGroup1       yes
        """
  Given I use repository "dnf-ci-group-rollback-2"
    And I successfully execute dnf with args "group upgrade dnf-ci-group-rollback-testgroup1"
   Then Transaction is following
        | Action        | Package                                       |
        | group-upgrade | DNF-CI-RollbackTestGroup1                     |
        | upgrade       | TestGroup1PackageA-0:1.1-1.x86_64             |
        | upgrade       | TestGroup1PackageB-0:1.1-1.x86_64             |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |


@bz2016070
Scenario: Rollback a group upgrade transaction
  Given I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | downgrade              | TestGroup1PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup1PackageB-0:1.0-1.x86_64    |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 3      | history rollback 1                                   |        | 4         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
   When I execute dnf with args "group list --installed dnf-ci-group-rollback-testgroup1"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ID                               Name                      Installed
        dnf-ci-group-rollback-testgroup1 DNF-CI-RollbackTestGroup1       yes
        """


@bz2016070
Scenario: Rollback a rollbacked group upgrade transaction
  Given I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | downgrade              | TestGroup1PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup1PackageB-0:1.0-1.x86_64    |
    And stderr contains lines
    """
    Group upgrade cannot be reverted, however associated package actions will be. (Group id: 'dnf-ci-group-rollback-testgroup1') .
    """
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 3      | history rollback 1                                   |        | 4         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
   When I execute dnf with args "history rollback 2"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | upgrade                | TestGroup1PackageA-0:1.1-1.x86_64    |
        | upgrade                | TestGroup1PackageB-0:1.1-1.x86_64    |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 4      | history rollback 2                                   |        | 4         |
        | 3      | history rollback 1                                   |        | 4         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
   When I execute dnf with args "group list --installed dnf-ci-group-rollback-testgroup1"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ID                               Name                      Installed
        dnf-ci-group-rollback-testgroup1 DNF-CI-RollbackTestGroup1       yes
        """


@bz2016070
Scenario: Redo an undo-ed group upgrade transaction
  Given I execute dnf with args "history undo last"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | downgrade              | TestGroup1PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup1PackageB-0:1.0-1.x86_64    |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 3      | history undo last                                    |        | 4         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
   When I execute dnf with args "history redo last"
   Then the exit code is 0


@bz2016070
Scenario: Rollback multiple group upgrade transactions
  Given I use repository "dnf-ci-group-rollback-3"
    And I successfully execute dnf with args "group upgrade dnf-ci-group-rollback-testgroup1"
   Then Transaction is following
        | Action        | Package                                       |
        | group-upgrade | DNF-CI-RollbackTestGroup1                     |
        | upgrade       | TestGroup1PackageA-0:1.2-1.x86_64             |
        | upgrade       | TestGroup1PackageB-0:1.2-1.x86_64             |
        | install-group | TestGroup1PackageC-0:1.0-1.x86_64             |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 3      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 6         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
  Given I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | downgrade              | TestGroup1PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup1PackageB-0:1.0-1.x86_64    |
        | remove                 | TestGroup1PackageC-0:1.0-1.x86_64    |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 4      | history rollback 1                                   |        | 5         |
        | 3      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 6         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
   When I execute dnf with args "group list --installed dnf-ci-group-rollback-testgroup1"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ID                               Name                      Installed
        dnf-ci-group-rollback-testgroup1 DNF-CI-RollbackTestGroup1       yes
        """


@bz2016070
Scenario: Excluded package is remembered until next group install when rolling back a group upgrade transaction
  Given I use repository "dnf-ci-group-rollback-3"
    And I successfully execute dnf with args "group upgrade dnf-ci-group-rollback-testgroup1 -x TestGroup1PackageC"
   Then Transaction is following
        | Action        | Package                                       |
        | group-upgrade | DNF-CI-RollbackTestGroup1                     |
        | upgrade       | TestGroup1PackageA-0:1.2-1.x86_64             |
        | upgrade       | TestGroup1PackageB-0:1.2-1.x86_64             |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 3      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
  Given I execute dnf with args "history rollback 2"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | downgrade              | TestGroup1PackageA-0:1.1-1.x86_64    |
        | downgrade              | TestGroup1PackageB-0:1.1-1.x86_64    |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 4      | history rollback 2                                   |        | 4         |
        | 3      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
  Given I successfully execute dnf with args "group upgrade dnf-ci-group-rollback-testgroup1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                       |
        | group-upgrade | DNF-CI-RollbackTestGroup1                     |
        | upgrade       | TestGroup1PackageA-0:1.2-1.x86_64             |
        | upgrade       | TestGroup1PackageB-0:1.2-1.x86_64             |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 5      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 4      | history rollback 2                                   |        | 4         |
        | 3      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
  Given I successfully execute dnf with args "group install dnf-ci-group-rollback-testgroup1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                       |
        | install-group | TestGroup1PackageC-0:1.0-1.x86_64             |
    And stderr contains "Group \"dnf-ci-group-rollback-testgroup1\" is already installed."
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 6      | group install dnf-ci-group-rollback-testgroup1       |        | 1         |
        | 5      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 4      | history rollback 2                                   |        | 4         |
        | 3      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 2      | group upgrade dnf-ci-group-rollback-testgroup1       |        | 5         |
        | 1      | group install dnf-ci-group-rollback-testgroup1       |        | 3         |
   When I execute dnf with args "group list --installed dnf-ci-group-rollback-testgroup1"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ID                               Name                      Installed
        dnf-ci-group-rollback-testgroup1 DNF-CI-RollbackTestGroup1       yes
        """
