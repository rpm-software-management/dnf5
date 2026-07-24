# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module repoquery command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Modular repoquery


Scenario Outline: <command> returns packages from both default and enabled streams
  Given I use repository "dnf-ci-fedora-modular"
    And I successfully execute dnf with args "module enable dwm:6.0"
   When I execute dnf with args "module <command> dwm nodejs"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        dwm-6.0-1.module_1997+c375c79c.src
        dwm-6.0-1.module_1997+c375c79c.x86_64
        nodejs-1:8.11.4-1.module_2030+42747d40.src
        nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
        nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
        nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch
        npm-1:8.11.4-1.module_2030+42747d40.x86_64
        """

Examples:
    | command                   |
    | repoquery                 |
    | repoquery --available     |


Scenario: module repoquery --installed lists only installed modular packages
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "module install nodejs:10/minimal"
   When I execute dnf with args "module repoquery --installed dwm nodejs"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64
        npm-1:10.11.0-1.module_2200+adbac02b.x86_64
        """


Scenario: module repoquery --installed returns also packages not available any more
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora-modular-updates"
    And I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "module install nodejs:12/minimal"
    And I drop repository "dnf-ci-fedora-modular-updates"
   When I execute dnf with args "module repoquery --installed dwm nodejs"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        nodejs-1:12.1.0-1.module_2379+8d497405.x86_64
        npm-1:12.1.0-1.module_2379+8d497405.x86_64
        """


Scenario: module repoquery can be used with both --installed and --available together
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora-modular-updates"
    And I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "module install nodejs:12/minimal"
    And I drop repository "dnf-ci-fedora-modular-updates"
   When I execute dnf with args "module repoquery --installed --available dwm nodejs ninja"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ninja-build-1.8.2-4.module_1991+4e5efe2f.src
        ninja-build-1.8.2-4.module_1991+4e5efe2f.x86_64
        ninja-build-debuginfo-1.8.2-4.module_1991+4e5efe2f.x86_64
        ninja-build-debugsource-1.8.2-4.module_1991+4e5efe2f.x86_64
        nodejs-1:12.1.0-1.module_2379+8d497405.x86_64
        npm-1:12.1.0-1.module_2379+8d497405.x86_64
        """
