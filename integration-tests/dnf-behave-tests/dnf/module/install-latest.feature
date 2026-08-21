# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Installing modules without profile specification using defaults from repo


Background:
  Given I use repository "dnf-ci-multicontext-hybrid-multiversion-modular"
    And I set default module platformid to "platform:f29"


@bz1781769
Scenario: Install module, profile from the latest module context
   When I execute dnf with args "module enable postgresql:9.6"
   Then the exit code is 0
    And modules state is following
        | Module        | State     | Stream    | Profiles  |
        | postgresql    | enabled   |    9.6    |           |
   When I execute dnf with args "module install nodejs:5/testlatest"
   Then the exit code is 0
    And modules state is following
        | Module   | State     | Stream    | Profiles   |
        | nodejs   |  enabled  |     5     | testlatest |
    And Transaction is following
        | Action                    | Package                                                                                         |
        | module-stream-enable      | nodejs:5                                                                                        |
        | module-profile-install    | nodejs/testlatest                                                                               |
        | install-group             | postgresql-0:9.6.8-1.module_1710+b535a823.x86_64 |
