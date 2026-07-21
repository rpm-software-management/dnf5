@xfail
# The plugin is missing: https://github.com/rpm-software-management/dnf5/issues/925
Feature: Test for debug plugin - restoring


Background: install some packages and create dump file
  Given I use repository "debug-plugin"
    And I successfully execute dnf with args "install kernel-4.19.1 kernel-4.20.1"
    And I successfully execute dnf with args "install test-replace-2"
    And I successfully execute dnf with args "debug-dump {context.dnf.tempdir}/dump.txt"


@bz1844533
Scenario: debug-restore does not do anything if there is no package set change
   When I execute dnf with args "debug-restore {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is empty
    And stderr is
    """
    <REPOSYNC>
    Complete!
    """
    And stdout is
    """
    Dependencies resolved.
    Nothing to do.
    """


Scenario: debug-restore can install missing packages in correct versions
  Given I successfully execute dnf with args "remove test-replace"
   When I execute dnf with args "debug-restore {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | test-replace-0:2-fc29.x86_64          |


Scenario: debug-restore --install-latest installs the latest version of missing package
  Given I successfully execute dnf with args "remove test-replace"
   When I execute dnf with args "debug-restore --install-latest {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | test-replace-0:3-fc29.x86_64          |


Scenario: debug-restore does not install missing packages if 'install' not in filter-types
  Given I successfully execute dnf with args "remove test-replace"
   When I execute dnf with args "debug-restore {context.dnf.tempdir}/dump.txt --filter-types=remove,replace"
   Then the exit code is 0
    And Transaction is empty
    And stderr is
    """
    <REPOSYNC>
    Complete!
    """
    And stdout is
    """
    Dependencies resolved.
    Nothing to do.
    """


Scenario: debug-restore can remove extra packages
  Given I successfully execute dnf with args "install test-remove"
   When I execute dnf with args "debug-restore {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | test-remove-0:1-fc29.x86_64           |


Scenario: debug-restore does not remove packages if 'remove' not in filter-types
  Given I successfully execute dnf with args "install test-remove"
   When I execute dnf with args "debug-restore {context.dnf.tempdir}/dump.txt --filter-types=install,replace"
   Then the exit code is 0
    And Transaction is empty
    And stderr is
    """
    <REPOSYNC>
    Complete!
    """
    And stdout is
    """
    Dependencies resolved.
    Nothing to do.
    """


Scenario: debug-restore can upgrade packages
  Given I successfully execute dnf with args "downgrade test-replace"
   When I execute dnf with args "debug-restore {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | upgrade       | test-replace-0:2-fc29.x86_64          |


Scenario: debug-restore can downgrade packages
  Given I successfully execute dnf with args "upgrade test-replace"
   When I execute dnf with args "debug-restore {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | downgrade     | test-replace-0:2-fc29.x86_64          |


Scenario: debug-restore does not replace packages if 'replace' not in filter-types
  Given I successfully execute dnf with args "downgrade test-replace"
   When I execute dnf with args "debug-restore {context.dnf.tempdir}/dump.txt --filter-types=install,remove"
   Then the exit code is 0
    And Transaction is empty
    And stderr is
    """
    <REPOSYNC>
    Complete!
    """
    And stdout is
    """
    <REPOSYNC>
    Dependencies resolved.
    Nothing to do.
    """

@bz1844533
Scenario: debug-restore does not remove install-only packages
  Given I successfully execute dnf with args "install kernel-4.18.1"
    And I successfully execute dnf with args "remove kernel-4.20.1"
   When I execute dnf with args "debug-restore {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-0:4.20.1-fc29.x86_64           |


@bz1844533
Scenario: debug-restore --remove-installonly does remove install-only packages
  Given I successfully execute dnf with args "install kernel-4.18.1"
    And I successfully execute dnf with args "remove kernel-4.20.1"
   When I execute dnf with args "debug-restore --remove-installonly {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-0:4.20.1-fc29.x86_64           |
        | remove        | kernel-0:4.18.1-fc29.x86_64           |


@bz1844533
@no_installroot
Scenario: debug-restore --remove-installonly fails to remove running kernel
  Given I successfully execute dnf with args "install kernel-4.18.1"
    And I successfully execute dnf with args "remove kernel-4.20.1"
    And I fake kernel release to "4.18.1-fc29.x86_64"
   When I execute dnf with args "debug-restore --remove-installonly {context.dnf.tempdir}/dump.txt"
   Then the exit code is 1
    And stderr is
    """
    Error:
     Problem: The operation would result in removing the following protected packages: kernel
    """


Scenario: debug-restore --output only prints what would be changed
  Given I successfully execute dnf with args "upgrade test-replace"
    And I successfully execute dnf with args "install kernel-4.18.1"
    And I successfully execute dnf with args "remove kernel-4.20.1"
   When I execute dnf with args "debug-restore --output {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is empty
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    install   kernel-0:4.20.1-fc29.x86_64
    replace   test-replace-0:2-fc29.x86_64
    """


Scenario: debug-restore --output only prints what would be changed (with --remove-installonly)
  Given I successfully execute dnf with args "upgrade test-replace"
    And I successfully execute dnf with args "install kernel-4.18.1"
    And I successfully execute dnf with args "remove kernel-4.20.1"
   When I execute dnf with args "debug-restore --remove-installonly --output {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And Transaction is empty
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    remove    kernel-0:4.18.1-fc29.x86_64
    install   kernel-0:4.20.1-fc29.x86_64
    replace   test-replace-0:2-fc29.x86_64
    """
