# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Disabling module stream


Background:
  Given I use repository "dnf-ci-fedora-modular"
  Given I use repository "dnf-ci-fedora"


@bz1677640
Scenario: I can disable a module when specifying module name
   When I execute dnf with args "module enable nodejs:8"
   Then the exit code is 0
   When I execute dnf with args "module disable nodejs"
   Then the exit code is 0
    And stdout contains "Disabling modules:"
    And Transaction is following
        | Action                    | Package           |
        | module-disable            | nodejs            |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | disabled  |           |           |


# this is not test for @bz1677640, but test is failing until the bug is fixed
Scenario: Disabling an already disabled module should pass
   When I execute dnf with args "module enable nodejs:8"
   Then the exit code is 0
   When I execute dnf with args "module disable nodejs"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package           |
        | module-disable            | nodejs            |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | disabled  |           |           |
   When I execute dnf with args "module disable nodejs"
   Then the exit code is 0
    And stdout contains "Nothing to do."


# Missing report about unneeded information
# https://github.com/rpm-software-management/dnf5/issues/1028
@xfail
@bz1649261
Scenario Outline: I can disable a module when specifying <spec>
   When I execute dnf with args "module enable nodejs:8"
   Then the exit code is 0
   When I execute dnf with args "module disable <modulespec>"
   Then the exit code is 0
    And stdout contains "Only module name is required. Ignoring unneeded information in argument: '<modulespec>'"
    And Transaction is following
        | Action                    | Package           |
        | module-disable            | nodejs            |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | disabled  |           |           |

Examples:
    | spec              | modulespec                |
    | stream            | nodejs:10                 |
    | version           | nodejs:10:20180920144631  |


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
# Missing report about unneeded information
# https://github.com/rpm-software-management/dnf5/issues/1028
@xfail
@bz1649261
Scenario Outline: I can disable a module with installed profile when specifying <spec>
   When I execute dnf with args "module install nodejs:10/default"
   Then the exit code is 0
   When I execute dnf with args "module disable <modulespec>"
   Then the exit code is 0
    And stdout contains "Only module name is required. Ignoring unneeded information in argument: '<modulespec>'"
    And Transaction is following
        | Action                    | Package           |
        | module-disable            | nodejs            |
        | module-profile-disable    | nodejs/default    |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | disabled  |           |           |

Examples:
    | spec              | modulespec                |
    | stream            | nodejs:10                 |
    | other stream      | nodejs:8                  |
    | version           | nodejs:10:20180920144631  |


@bz1613910
Scenario: It is possible to disable an enabled default stream
   When I execute dnf with args "module enable nodejs"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |
   When I execute dnf with args "module disable nodejs"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | disabled  |           |           |
   When I execute dnf with args "module list nodejs"
   Then the exit code is 0
    And module list contains
        | Name          | Stream       | Profiles                          |
        | nodejs        | 8 [d][x]     | development, minimal, default [d] |


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
@bz1653623
@bz1583596
Scenario: User is informed about disabling installed profiles when disabling a module
   When I execute dnf with args "module install nodejs:10/default nodejs:10/development"
   Then the exit code is 0
   When I execute dnf with args "module install postgresql:9.6/client"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles              |
        | nodejs    | enabled   | 10        | development, default  |
        | postgresql| enabled   | 9.6       | client                |
   # disabling the module removes its installed profiles and user is informed
   # in the transaction table
   When I execute dnf with args "module disable nodejs"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package               |
        | module-disable            | nodejs                |
        | module-profile-disable    | nodejs/default        |
        | module-profile-disable    | nodejs/development    |
    And modules state is following
        | Module    | State     | Stream    | Profiles              |
        | nodejs    | disabled  |           |                       |
        | postgresql| enabled   | 9.6       | client                |
   # after enabling nodejs:10 again, there are no profiles installed
   When I execute dnf with args "module enable nodejs:10"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package               |
        | module-stream-enable      | nodejs:10             |
    And modules state is following
        | Module    | State     | Stream    | Profiles              |
        | nodejs    | enabled   | 10        |                       |
        | postgresql| enabled   | 9.6       | client                |
