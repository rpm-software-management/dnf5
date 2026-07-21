# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Enable module requires confirmation


Background: Do not assume yes
  Given I do not assume yes
    And I use repository "dnf-ci-fedora-modular"


Scenario: Enable a module stream with --assumeyes
   When I execute dnf with args "module enable nodejs:8 --assumeyes"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-enable     | nodejs:8           |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |


Scenario: Enable a module stream with -y
   When I execute dnf with args "module enable nodejs:8 -y"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-enable     | nodejs:8           |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |


Scenario: Abort enabling of a module stream with --assumeno
   When I execute dnf with args "module enable nodejs:8 --assumeno"
   Then the exit code is 1
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    |           |           |           |
    And stderr contains "Operation aborted."
