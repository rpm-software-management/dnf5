# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Default streams are properly switched to enabled

Background:
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"


@bz1657213
Scenario: The default stream is enabled when requiring module is enabled
   When I execute dnf with args "module enable meson:master"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | meson     | enabled   | master    |           |
        | ninja     | enabled   | master    |           |


# package morning-mood is part of DnfCiModulePackageDep module and requires nodejs package (part of nodejs module)
# DnfCiModulePackageDep:packagedep does not have any modular dependencies
# DnfCiModulePackageDep:moduledep requires nodejs module


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1811
Scenario Outline: The default stream is enabled when its package is required by installed package of another module <description>
  Given I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "module enable DnfCiModulePackageDep:<stream>"
   Then the exit code is 0
   When I execute dnf with args "install morning-mood"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | install                   | morning-mood-0:1.0-1.module.x86_64            |
        | install-dep               | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
    And modules state is following
        | Module                | State     | Stream        | Profiles  |
        | DnfCiModulePackageDep | enabled   | <stream>      |           |
        | nodejs                | enabled   | 8             |           |

  Examples:
        | description           | stream        |
        | (no module deps)      | packagedep    |
        | (module deps set)     | moduledep     |


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1811
Scenario: The default stream is enabled when its package is required by installed non-modular package
  Given I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install anitras-dance"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | install                   | anitras-dance-0:1.0-1.x86_64                  |
        | install-dep               | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
    And modules state is following
        | Module                | State     | Stream        | Profiles  |
        | nodejs                | enabled   | 8             |           |
