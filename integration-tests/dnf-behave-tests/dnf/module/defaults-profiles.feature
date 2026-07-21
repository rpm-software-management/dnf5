# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Non-default profiles can be installed when explicitly specified on command line


Background:
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"

# module nodejs:
#   streams: 8 [d], 10
#   each stream has profiles: default [d], development, minimal

Scenario: I can install a non-default profile using dnf module install module:stream/profile
   When I execute dnf with args "module install nodejs:10/minimal"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                           |
        | install-group             | nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64    |
        | module-profile-install    | nodejs/minimal                                    |
        | module-stream-enable      | nodejs:10                                         |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 10        | minimal   |


@bz1573831
Scenario: I can install a non-default profile from a default stream using dnf module install module/profile
   When I execute dnf with args "module install nodejs/minimal"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                           |
        | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64     |
        | module-profile-install    | nodejs/minimal                                    |
        | module-stream-enable      | nodejs:8                                          |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         | minimal   |


Scenario: I can install a non-default profile from an enabled stream using dnf module install module/profile
   When I execute dnf with args "module enable nodejs:10"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 10        |           |
   When I execute dnf with args "module install nodejs/minimal"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                           |
        | install-group             | nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64    |
        | module-profile-install    | nodejs/minimal                                    |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 10        | minimal   |
