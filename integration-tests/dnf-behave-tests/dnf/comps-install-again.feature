Feature: Test installing groups and environments that are already installed


Scenario: Install a group that is already installed
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install C-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | C-mandatory-0:1.0-1.x86_64        |
        | group-install | C-group                           |
   When I execute dnf with args "group install C-group"
   Then the exit code is 0
    And Transaction is empty
    And stderr contains "Group \"C-group\" is already installed."


Scenario: Install an environment that is already installed
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install AB-environment"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | A-mandatory-0:1.0-1.x86_64        |
        | install-group | A-default-0:1.0-1.x86_64          |
        | group-install | A-group - repo#1                  |
        | env-install   | AB-environment                    |
   When I execute dnf with args "group install AB-environment"
   Then the exit code is 0
    And Transaction is empty
    And stderr contains "Environmental group \"AB-environment\" is already installed."
    And stderr contains "Group \"a-group\" is already installed."


Scenario: Install a group that is already installed, but with different definition
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install AB-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | A-mandatory-0:1.0-1.x86_64        |
        | install-group | A-default-0:1.0-1.x86_64          |
        | group-install | AB-group                          |
   When I use repository "comps-upgrade-2"
    And I execute dnf with args "group install AB-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | upgrade       | A-mandatory-0:2.0-1.x86_64        |
        | upgrade       | A-default-0:2.0-1.x86_64          |
        | install-group | B-mandatory-0:1.0-1.x86_64        |
        | install-group | B-default-0:1.0-1.x86_64          |
    And stderr contains "Group \"AB-group\" is already installed."


Scenario: Install an environment that is already installed, but with different definition
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install AB-environment"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | A-mandatory-0:1.0-1.x86_64        |
        | install-group | A-default-0:1.0-1.x86_64          |
        | group-install | A-group - repo#1                  |
        | env-install   | AB-environment                    |
   When I use repository "comps-upgrade-2"
    And I execute dnf with args "group install AB-environment"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | upgrade       | A-mandatory-0:2.0-1.x86_64        |
        | upgrade       | A-default-0:2.0-1.x86_64          |
        | install-group | B-mandatory-0:1.0-1.x86_64        |
        | install-group | B-default-0:1.0-1.x86_64          |
        | group-install | B-group                           |
    And stderr contains "Environmental group \"AB-environment\" is already installed."


Scenario: Install a group that is already installed, but only as a dependency -> change the reason
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install AB-environment"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | A-mandatory-0:1.0-1.x86_64        |
        | install-group | A-default-0:1.0-1.x86_64          |
        | group-install | A-group - repo#1                  |
        | env-install   | AB-environment                    |
    And I execute dnf with args "group install a-group"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                           |
        | group-changing-reason | A-group - repo#1                  |
    And dnf5 transaction items for transaction "last" are
        | action        | package       | reason     | repository          |
        | Reason Change | a-group       | User       | comps-upgrade-1     |


Scenario: Install an environment that contains an already installed group -> don't change the reason
  Given I use repository "comps-upgrade-1"
    And I execute dnf with args "group install a-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | A-mandatory-0:1.0-1.x86_64        |
        | install-group | A-default-0:1.0-1.x86_64          |
        | group-install | A-group - repo#1                  |
   When I execute dnf with args "group install AB-environment"
   Then the exit code is 0
    And Transaction is following
        | Action           | Package                           |
        | env-install      | AB-environment                    |
    And stderr contains "Group \"a-group\" is already installed."
