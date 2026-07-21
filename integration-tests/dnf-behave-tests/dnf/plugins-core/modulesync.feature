@xfail
# The plugin is missing: https://github.com/rpm-software-management/dnf5/issues/932
@1868047
Feature: Test for modulesync command


Background:
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"


Scenario: I can download a module
  Given I create directory "modulesync"
    And I set working directory to "{context.dnf.installroot}/modulesync"
    And I execute dnf with args "modulesync nodejs:8/minimal"
   Then the exit code is 0
    And file "modulesync/nodejs-8.11.4-1.module_2030+42747d40.src.rpm" exists
    And file "modulesync/nodejs-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "modulesync/nodejs-devel-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "modulesync/nodejs-docs-8.11.4-1.module_2030+42747d40.noarch.rpm" exists
    And file "modulesync/npm-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Directory walk started
    Directory walk done - 5 packages
    Loaded information about 0 packages
    Temporary output repo path: {context.dnf.installroot}/modulesync/.repodata/
    Preparing sqlite DBs
    Pool started (with 5 workers)
    Pool finished
    """


Scenario: I can download a module using --downloaddir
  Given I create directory "modulesync"
    And I use repository "dnf-ci-fedora-modular-updates"
    And I execute dnf with args "modulesync nodejs:8/minimal --downloaddir={context.dnf.installroot}/modulesync"
   Then the exit code is 0
    And file "modulesync/nodejs-8.11.4-1.module_2030+42747d40.src.rpm" exists
    And file "modulesync/nodejs-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "modulesync/nodejs-devel-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "modulesync/nodejs-docs-8.11.4-1.module_2030+42747d40.noarch.rpm" exists
    And file "modulesync/npm-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Directory walk started
    Directory walk done - 6 packages
    Loaded information about 0 packages
    Temporary output repo path: {context.dnf.installroot}/modulesync/.repodata/
    Preparing sqlite DBs
    Pool started (with 5 workers)
    Pool finished
    """



Scenario: I can download a module with option newest only
  Given I use repository "dnf-ci-fedora-modular-updates"
    And I create directory "modulesync"
    And I execute dnf with args "modulesync nodejs:8/minimal --newest-only --downloaddir={context.dnf.installroot}/modulesync"
   Then the exit code is 0
    And file "modulesync/nodejs-8.11.4-1.module_2030+42747d40.src.rpm" exists
    And file "modulesync/nodejs-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "modulesync/nodejs-devel-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "modulesync/nodejs-docs-8.11.4-1.module_2030+42747d40.noarch.rpm" exists
    And file "modulesync/npm-8.14.0-1.module_2030+42747d41.x86_64.rpm" exists
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Directory walk started
    Directory walk done - 5 packages
    Loaded information about 0 packages
    Temporary output repo path: {context.dnf.installroot}/modulesync/.repodata/
    Preparing sqlite DBs
    Pool started (with 5 workers)
    Pool finished
    """


Scenario: I can download a module and init a modular repository with modulesync
  Given I create directory "modulesync"
    And I successfully execute dnf with args "install nodejs --downloadonly --downloaddir={context.dnf.installroot}/modulesync"
    And file "modulesync/basesystem-11-6.fc29.noarch.rpm" exists
    And file "modulesync/filesystem-3.9-2.fc29.x86_64.rpm" exists
    And file "modulesync/glibc-2.28-9.fc29.x86_64.rpm" exists
    And file "modulesync/glibc-all-langpacks-2.28-9.fc29.x86_64.rpm" exists
    And file "modulesync/glibc-common-2.28-9.fc29.x86_64.rpm" exists
    And file "modulesync/nodejs-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "modulesync/npm-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "modulesync/setup-2.12.1-1.fc29.noarch.rpm" exists
   When I execute dnf with args "modulesync --downloaddir={context.dnf.installroot}/modulesync"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Directory walk started
    Directory walk done - 8 packages
    Loaded information about 0 packages
    Temporary output repo path: {context.dnf.installroot}/modulesync/.repodata/
    Preparing sqlite DBs
    Pool started (with 5 workers)
    Pool finished
    """


Scenario: I can create a repository with modulesync in a custom dir and install:
  Given I create directory "modulesync"
    And I successfully execute dnf with args "modulesync nodejs:8/minimal --destdir={context.dnf.installroot}/modulesync --resolve"
   When I execute dnf with args "module install nodejs:8/minimal --nogpgcheck --disablerepo='*' --repofrompath=tmp,{context.dnf.installroot}/modulesync"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                       |
        | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
        | module-profile-install    | nodejs/minimal                                |
        | module-stream-enable      | nodejs:8                                      |
    And stdout contains "Installing group/module packages"
