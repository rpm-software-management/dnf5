Feature: Transaction history rollback of environment upgrades


Background:
  Given I use repository "dnf-ci-group-rollback-1"
    And I successfully execute dnf with args "group install dnf-ci-group-rollback-testenv"
   Then Transaction is following
        | Action        | Package                                       |
        | env-install   | DNF-CI-RollbackTestEnv                        |
        | group-install | DNF-CI-RollbackTestGroup1                     |
        | group-install | DNF-CI-RollbackTestGroup2                     |
        | install-group | TestGroup1PackageA-0:1.0-1.x86_64             |
        | install-group | TestGroup1PackageB-0:1.0-1.x86_64             |
        | install-group | TestGroup2PackageA-0:1.0-1.x86_64             |
        | install-group | TestGroup2PackageB-0:1.0-1.x86_64             |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 1      | group install dnf-ci-group-rollback-testenv          |        | 7         |
   When I execute dnf with args "environment list --installed dnf-ci-group-rollback-testenv"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ID                            Name                   Installed
        dnf-ci-group-rollback-testenv DNF-CI-RollbackTestEnv       yes
        """
  Given I use repository "dnf-ci-group-rollback-2"
    And I successfully execute dnf with args "group upgrade dnf-ci-group-rollback-testenv"
   Then Transaction is following
        | Action        | Package                                       |
        | env-upgrade   | DNF-CI-RollbackTestEnv                        |
        | group-upgrade | DNF-CI-RollbackTestGroup1                     |
        | group-upgrade | DNF-CI-RollbackTestGroup2                     |
        | upgrade       | TestGroup1PackageA-0:1.1-1.x86_64             |
        | upgrade       | TestGroup1PackageB-0:1.1-1.x86_64             |
        | upgrade       | TestGroup2PackageA-0:1.1-1.x86_64             |
        | upgrade       | TestGroup2PackageB-0:1.1-1.x86_64             |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 2      | group upgrade dnf-ci-group-rollback-testenv          |        | 11        |
        | 1      | group install dnf-ci-group-rollback-testenv          |        | 7         |


@bz2016070
Scenario: Rollback an environment upgrade transaction
  Given I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | downgrade              | TestGroup1PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup1PackageB-0:1.0-1.x86_64    |
        | downgrade              | TestGroup2PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup2PackageB-0:1.0-1.x86_64    |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 3      | history rollback 1                                   |        | 8         |
        | 2      | group upgrade dnf-ci-group-rollback-testenv          |        | 11        |
        | 1      | group install dnf-ci-group-rollback-testenv          |        | 7         |
   When I execute dnf with args "environment list --installed dnf-ci-group-rollback-testenv"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ID                            Name                   Installed
        dnf-ci-group-rollback-testenv DNF-CI-RollbackTestEnv       yes
        """


@bz2016070
Scenario: Rollback a rollbacked environment upgrade transaction
  Given I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | downgrade              | TestGroup1PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup1PackageB-0:1.0-1.x86_64    |
        | downgrade              | TestGroup2PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup2PackageB-0:1.0-1.x86_64    |
    And stderr contains lines
        """
        Group upgrade cannot be reverted, however associated package actions will be. (Group id: 'dnf-ci-group-rollback-testgroup1') .
        Group upgrade cannot be reverted, however associated package actions will be. (Group id: 'dnf-ci-group-rollback-testgroup2') .
        Environment upgrade cannot be reverted, however associated package actions will be. (Environment id: 'dnf-ci-group-rollback-testenv') .
        """
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 3      | history rollback 1                                   |        | 8         |
        | 2      | group upgrade dnf-ci-group-rollback-testenv          |        | 11        |
        | 1      | group install dnf-ci-group-rollback-testenv          |        | 7         |
   When I execute dnf with args "history rollback 2"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | upgrade                | TestGroup1PackageA-0:1.1-1.x86_64    |
        | upgrade                | TestGroup1PackageB-0:1.1-1.x86_64    |
        | upgrade                | TestGroup2PackageA-0:1.1-1.x86_64    |
        | upgrade                | TestGroup2PackageB-0:1.1-1.x86_64    |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 4      | history rollback 2                                   |        | 8         |
        | 3      | history rollback 1                                   |        | 8         |
        | 2      | group upgrade dnf-ci-group-rollback-testenv          |        | 11        |
        | 1      | group install dnf-ci-group-rollback-testenv          |        | 7         |
   When I execute dnf with args "environment list --installed DNF-CI-RollbackTestEnv"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ID                            Name                   Installed
        dnf-ci-group-rollback-testenv DNF-CI-RollbackTestEnv       yes
        """


@bz2016070
Scenario: Redo an undo-ed environment upgrade transaction
  Given I execute dnf with args "history undo last"
   Then the exit code is 0
    And Transaction is following
        | Action                 | Package                              |
        | downgrade              | TestGroup1PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup1PackageB-0:1.0-1.x86_64    |
        | downgrade              | TestGroup2PackageA-0:1.0-1.x86_64    |
        | downgrade              | TestGroup2PackageB-0:1.0-1.x86_64    |
    And History is following
        | Id     | Command                                              | Action | Altered   |
        | 3      | history undo last                                    |        | 8         |
        | 2      | group upgrade dnf-ci-group-rollback-testenv          |        | 11        |
        | 1      | group install dnf-ci-group-rollback-testenv          |        | 7         |
   When I execute dnf with args "history redo last"
   Then the exit code is 0
