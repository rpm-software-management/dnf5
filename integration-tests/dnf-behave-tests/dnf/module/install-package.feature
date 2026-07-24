# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Installing package from module


Scenario: I can install a specific package from a module
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
    And I execute dnf with args "module enable ninja:master"
   When I execute dnf with args "install ninja-build"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                           |
        | install                   | ninja-build-0:1.8.2-4.module_1991+4e5efe2f.x86_64 |


Scenario: I can install a package from modular repo not belonging to a module
  Given I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install solveigs-song"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                           |
        | install                   | solveigs-song-0:1.0-1.x86_64      |


Scenario: I cannot install a specific package from not enabled module when default stream is not defined
  Given I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install arabian-dance"
   Then the exit code is 1
    And stderr is
    """
    <REPOSYNC>
    Failed to resolve the transaction:
    Argument 'arabian-dance' matches only excluded packages.
    """


@xfail
# Missing shell command https://github.com/rpm-software-management/dnf5/issues/153
@bz1767351
Scenario: I cannot install a specific package from not enabled module after reset
  Given I use repository "dnf-ci-multicontext-modular"
   When I execute dnf with args "install nodejs"
   Then Transaction is empty
   When I execute dnf with args "module enable nodejs:5"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-enable     | nodejs:5           |
        | module-stream-enable     | postgresql:9.6     |
    And modules state is following
        | Module     | State     | Stream    | Profiles  |
        | nodejs     | enabled   | 5         |           |
        | postgresql | enabled   | 9.6       |           |
   When I open dnf shell session
    And I execute in dnf shell "module reset '*'"
#  nodejs must be after reset not available
    And I execute in dnf shell "install nodejs"
 #  Then stderr contains "Error: Unable to find a match"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action                   | Package            |
        | module-reset             | nodejs             |
        | module-reset             | postgresql         |
    And modules state is following
        | Module     | State     | Stream    | Profiles  |
        | nodejs     |           |           |           |
        | postgresql |           |           |           |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"

# module ninja:master [d] contains ninja-build-0:1.8.2-4.module_1991+4e5efe2f.x86_64
# ninja:legacy contains ninja-build-0:1.5.2-1.module_1991+4e5efe2f.x86_64
# ninja:development contans ninja-build-1.9.2-1.module_1991+4e5efe2f.x86_64
# ursine repo contains ninja-build-0:1.8.2-5.fc29.x86_64

Scenario: module content masks ursine content - module not enabled, default stream exists
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "install ninja-build-0:1.8.2-5.fc29.x86_64"
   Then the exit code is 1
    And stderr is
    """
    <REPOSYNC>
    Failed to resolve the transaction:
    Argument 'ninja-build-0:1.8.2-5.fc29.x86_64' matches only excluded packages.
    """


Scenario: module content masks ursine content - non-default stream enabled
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "module enable ninja:development"
   Then the exit code is 0
   When I execute dnf with args "install ninja-build-0:1.8.2-5.fc29.x86_64"
   Then the exit code is 1
    And stderr is
    """
    <REPOSYNC>
    Failed to resolve the transaction:
    Argument 'ninja-build-0:1.8.2-5.fc29.x86_64' matches only excluded packages.
    """


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1811
Scenario: a package from a non-enabled module is preferred when default stream is defined
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "install ninja-build"
   Then the exit code is 0
    And Transaction contains
        | Action                | Package                                           |
        | install               | ninja-build-0:1.8.2-4.module_1991+4e5efe2f.x86_64 |
        | module-stream-enable  | ninja:master                                      |
   When I execute dnf with args "module list --installed ninja"
   Then the exit code is 1
    And stderr contains "Error: No matching Modules to list"


Scenario: rpm from enabled stream is preferred regardless of NVRs
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "module enable ninja:legacy"
   Then the exit code is 0
   When I execute dnf with args "install ninja-build"
   Then the exit code is 0
    And Transaction contains
        | Action                | Package                                           |
        | install               | ninja-build-0:1.5.2-1.module_1991+4e5efe2f.x86_64 |


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1811
@bz1762314
Scenario: I can install a specific package from a module and enable all module dependencies
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
    And I execute dnf with args "install meson-doc"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                           |
        | install                   | meson-doc-0:0.47.1-5.module_1993+7c0a4d1e.noarch |
    And modules state is following
        | Module   | State     | Stream    | Profiles  |
        | meson    | enabled   | master    |           |
        | ninja    | enabled   | master    |           |
