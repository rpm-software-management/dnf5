# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
@not.with_os=rhel__eq__8
Feature: Handling new stream with multicontext modules


Background:
  Given I set default module platformid to "platform:f29"

Scenario: Enable new stream with multicontext stream enabled
  Given I use repository "dnf-ci-multicontext-hybrid-multiversion-modular"
    And I use repository "dnf-ci-multicontext-hybrid-multiversion-modular-updates"
   When I execute dnf with args "module enable postgresql:9.10"
   Then the exit code is 0
    And modules state is following
        | Module        | State     | Stream    | Profiles  |
        | postgresql    | enabled   |    9.10   |           |
   When I execute dnf with args "module install nodejs:5/minimal"
   Then the exit code is 0
    And modules state is following
        | Module   | State     | Stream    | Profiles  |
        | nodejs   |  enabled  |     5     | minimal   |
    And Transaction is following
        | Action                    | Package                                      |
        | module-stream-enable      | nodejs:5                                     |
        | module-profile-install    | nodejs/minimal                               |
        | install-group             | nodejs-1:5.4.1-3.module_1010+10787ba5.x86_64 |
