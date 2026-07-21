# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: hotfix repo content is not masked by a modular content


Background:
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
    And I configure repository "dnf-ci-fedora-modular-hotfix" with
        | key             | value |
        | module_hotfixes | 1     |


# dnf-ci-fedora-modular:        nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
# dnf-ci-fedora-modular-hotfix: nodejs-1:8.11.5-1.module_2030+42747d40.x86_64

# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
@bz1654738
Scenario: hotfix content updates are used when installing a module stream
  Given I use repository "dnf-ci-fedora-modular-hotfix"
   When I execute dnf with args "module install nodejs:8/minimal"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | module-stream-enable      | nodejs:8                                      |
        | module-profile-install    | nodejs/minimal                                |
        | install-group             | nodejs-1:8.11.5-1.module_2030+42747d40.x86_64 |


Scenario: hotfix content update is used when installing a package
  Given I use repository "dnf-ci-fedora-modular-hotfix"
   When I execute dnf with args "module enable nodejs:8"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                       |
        | module-stream-enable      | nodejs:8                                      |
   When I execute dnf with args "install nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | install                   | nodejs-1:8.11.5-1.module_2030+42747d40.x86_64 |


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Scenario: hotfix content updates are used for updating a module
   When I execute dnf with args "module install nodejs:8/minimal"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
  Given I use repository "dnf-ci-fedora-modular-hotfix"
   When I execute dnf with args "module update nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | upgrade                   | nodejs-1:8.11.5-1.module_2030+42747d40.x86_64 |


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Scenario: hotfix content is used when listing available updates
   When I execute dnf with args "module install nodejs:8/minimal"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-modular-hotfix"
   When I execute dnf with args "check-update"
   Then the exit code is 100
    And stdout contains "nodejs\.x86_64\s+1:8\.11\.5-1\.module_2030\+42747d40\s+dnf-ci-fedora-modular-hotfix"


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Scenario: hotfix content updates are used for updating a system
   When I execute dnf with args "module install nodejs:8/minimal"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-modular-hotfix"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | upgrade                   | nodejs-1:8.11.5-1.module_2030+42747d40.x86_64 |
