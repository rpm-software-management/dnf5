# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Installing modules without profile specification using defaults from repo


Background:
  Given I use repository "dnf-ci-multicontext-modular"

Scenario: Install module, with specified profile and enabled dependencies
   When I execute dnf with args "module enable postgresql:9.6"
   Then the exit code is 0
    And modules state is following
        | Module       | State     | Stream    | Profiles  |
        | postgresql   | enabled   | 9.6       |           |
   When I execute dnf with args "module install nodejs:5/minimal"
   Then the exit code is 0
    And modules state is following
        | Module         | State     | Stream   | Profiles  |
        | nodejs         | enabled   | 5        | minimal   |
    And Transaction is following
        | Action                    | Package                                         |
        | install-group             | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64    |
        | module-stream-enable      | nodejs:5                                        |
        | module-profile-install    | nodejs/minimal                                  |

@bz1696204
Scenario: Install module, with specified context that is not active (unsatisfied require)
   When I execute dnf with args "module enable postgresql:9.6"
   Then the exit code is 0
    And modules state is following
        | Module       | State     | Stream    | Profiles  |
        | postgresql   | enabled   | 9.6       |           |
   When I execute dnf with args "module install nodejs:5:20180801080000:7a1f8c5/minimal"
   Then the exit code is 1
    And modules state is following
        | Module         | State     | Stream   | Profiles  |
        | nodejs         |           |          |           |
    And stderr contains "All matches for argument 'nodejs:5:20180801080000:7a1f8c5/minimal' in module 'nodejs:5' are not active"
