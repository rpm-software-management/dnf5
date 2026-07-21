# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Installing module profiles with globs


Background:
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"


Scenario: Install a module profile with glob in name
   When I execute dnf with args "module install node*"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | module-stream-enable      | nodejs:8                                      |
        | module-profile-install    | nodejs/default                                |
        | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |


Scenario: Install a module profile with glob in name and stream
   When I execute dnf with args "module install node*:*0"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                           |
        | module-stream-enable      | nodejs:10                                         |
        | module-profile-install    | nodejs/default                                    |
        | install-group             | nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64    |


Scenario: Install a module profile with glob in version
  Given I use repository "dnf-ci-fedora-modular-updates"
   When I execute dnf with args "module install nodejs:10:20180920*"
   # By selecting module version that is not the latest,
   # older nodejs package from dnf-ci-fedora-modular gets installed.
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                           |
        | module-stream-enable      | nodejs:10                                         |
        | module-profile-install    | nodejs/default                                    |
        | install-group             | nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64    |


Scenario: Install a module profile with glob in arch
   When I execute dnf with args "module install nodejs::x86*"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | module-stream-enable      | nodejs:8                                      |
        | module-profile-install    | nodejs/default                                |
        | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |


Scenario: Install a module profile with glob in profile
   When I execute dnf with args "module install nodejs/*"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | module-stream-enable      | nodejs:8                                      |
        | module-profile-install    | nodejs/default                                |
        | module-profile-install    | nodejs/development                            |
        | module-profile-install    | nodejs/minimal                                |
