Feature: Transaction history rollback of broken history


Background: Add first transaction we can rollback to
  Given I use repository "dnf-ci-fedora"
    # Since we cannot rollback to before there were any transactions do first arbitrary transaction
    And I successfully execute dnf with args "install basesystem"
    And I use repository "simple-base"


Scenario: Rollback two installs of the same package
  Given I successfully execute dnf with args "install labirinto"
    And I successfully execute rpm with args "-e labirinto"
    And I successfully execute dnf with args "install labirinto"
   When I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action | Package                  |
        | remove | labirinto-1.0-1.fc29.x86_64 |
    And stderr contains "Transaction merge error: 'Action 'Remove' 'labirinto-1.0-1.fc29.x86_64' cannot be merged after it was 'Remove' in preceding transaction -> setting 'Remove'.'"


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/2223
Scenario: Rollback two installs and reinstall of the same package
  Given I successfully execute dnf with args "install labirinto"
    And I successfully execute dnf with args "reinstall labirinto"
    And I successfully execute rpm with args "-e labirinto"
    And I successfully execute dnf with args "install labirinto"
   When I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action | Package                     |
        | remove | labirinto-1.0-1.fc29.x86_64 |
    And stderr contains "Transaction merge error: 'Action 'Remove' 'labirinto-1.0-1.fc29.x86_64' cannot be merged after it was 'Remove' in preceding transaction -> setting 'Remove'.'"


Scenario: Rollback two removes of the same package
  Given I successfully execute dnf with args "install labirinto"
    And I successfully execute dnf with args "remove labirinto"
    And I successfully execute rpm with args "-i {context.dnf.fixturesdir}/repos/simple-base/x86_64/labirinto-1.0-1.fc29.x86_64.rpm"
    And I successfully execute dnf with args "remove labirinto"
   When I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is empty
    And stderr contains "Transaction merge error: 'Action 'Install' 'labirinto-1.0-1.fc29.x86_64' cannot be merged after it was 'Install' in preceding transaction -> setting 'Install'.'"
   When I execute dnf with args "history rollback 2"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                  |
        | install | labirinto-1.0-1.fc29.x86_64 |
    And stderr contains "Transaction merge error: 'Action 'Install' 'labirinto-1.0-1.fc29.x86_64' cannot be merged after it was 'Install' in preceding transaction -> setting 'Install'.'"


Scenario: Rollback install of a non-installonly package installed in multiple versions
  Given I successfully execute dnf with args "install labirinto"
    And I use repository "simple-updates"
    And I successfully execute dnf with args "update labirinto"
    # Simulate that the update failed to remove the old labirinto version
    And I successfully execute rpm with args "-i --force {context.dnf.fixturesdir}/repos/simple-base/x86_64/labirinto-1.0-1.fc29.x86_64.rpm"
   When I execute dnf with args "history rollback 2"
   Then the exit code is 0
    And Transaction is following
        | Action | Package                     |
        | remove | labirinto-2.0-1.fc29.x86_64 |
    And stderr contains "Transaction merge error: 'Action 'Install' 'labirinto-1.0-1.fc29.x86_64' cannot be merged because it is already present at that point -> skipping it.'"


Scenario: Rollback installs of a non-installonly package installed in multiple versions
  Given I successfully execute dnf with args "install labirinto"
    And I use repository "simple-updates"
    And I successfully execute dnf with args "update labirinto"
    # Simulate that the update failed to remove the old labirinto version
    And I successfully execute rpm with args "-i --force {context.dnf.fixturesdir}/repos/simple-base/x86_64/labirinto-1.0-1.fc29.x86_64.rpm"
   When I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And Transaction is following
        | Action | Package                     |
        | remove | labirinto-1.0-1.fc29.x86_64 |
        | remove | labirinto-2.0-1.fc29.x86_64 |
    And stderr contains "Transaction merge error: 'Action 'Install' 'labirinto-1.0-1.fc29.x86_64' cannot be merged because it is already present at that point -> skipping it.'"
