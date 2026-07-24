# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module switch-to command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Switch module to streams

Background:
Given I use repository "dnf-ci-fedora-modular"

Scenario Outline: Enable a module stream by <modulespec-type> using switch-to subcommand
   When I execute dnf with args "module switch-to <modulespec>"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-enable     | nodejs:8           |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |

Examples:
  | modulespec-type                 | modulespec                    |
  | module:stream                   | nodejs:8                      |
  | module:stream:version           | nodejs:8:20180801080000       |
  | glob                            | node*                         |
  | glob:glob                       | node*:*                       |
  | glob:glob:glob                  | node*:*:*0801*                |

@bz1792020
@bz1809314
Scenario: Switch a module to stream that was only enabled
   When I execute dnf with args "module enable nodejs:10"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-enable     | nodejs:10          |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 10        |           |
   When I execute dnf with args "module switch-to nodejs:8"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-switch     | nodejs:10 -> 8     |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |

@bz1792020
@bz1809314
Scenario: I can switch a module to stream, install packages from a new module and keep installed profile
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "module install nodejs:8/minimal"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
        | module-profile-install    | nodejs/minimal                                |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         | minimal   |
   When I execute dnf with args "module switch-to nodejs:10"
   Then the exit code is 0
    And Transaction contains
        | Action                   | Package                                       |
        | upgrade                  | nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64 |
        | upgrade                  | npm-1:10.11.0-1.module_2200+adbac02b.x86_64 |
        | module-stream-switch     | nodejs:8 -> 10     |
    And modules state is following
        | Module    | State     | Stream    | Profiles       |
        | nodejs    | enabled   | 10        |  minimal       |
   When I execute dnf with args "module switch-to nodejs:8"
   Then the exit code is 0
    And Transaction contains
        | Action                   | Package                                       |
        | downgrade                | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
        | downgrade                | npm-1:8.11.4-1.module_2030+42747d40.x86_64 |
        | module-stream-switch     | nodejs:10 -> 8     |
    And modules state is following
        | Module    | State     | Stream    | Profiles       |
        | nodejs    | enabled   | 8         |  minimal       |

@bz1792020
Scenario: Reject switch a module stream, when stream does not exist
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "module install nodejs:8/minimal"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
        | module-profile-install    | nodejs/minimal                                |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         | minimal   |
   When I execute dnf with args "module switch-to nodejs:notexists"
   Then the exit code is 1
    And Transaction is empty
    And modules state is following
        | Module    | State     | Stream    | Profiles       |
        | nodejs    | enabled   | 8         |  minimal       |
    And stderr is
        """
        Error: Problems in request:
        missing groups or modules: nodejs:notexists
        """
