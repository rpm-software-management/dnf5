Feature: Transaction history redo


Background:
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | filesystem-0:3.9-2.fc29.x86_64             |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
   When I execute dnf with args "remove filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | remove        | filesystem-0:3.9-2.fc29.x86_64             |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch               |


Scenario: Redo last transaction
   When I execute dnf with args "history redo last-1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install       | filesystem-0:3.9-2.fc29.x86_64             |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
    And History is following
        | Id     | Command               | Action | Altered   |
        | 3      |                       |        | 2         |
        | 2      |                       |        | 2         |
        | 1      | install filesystem    |        | 2         |
   When I execute dnf with args "history redo last-1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | remove        | filesystem-0:3.9-2.fc29.x86_64             |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch               |
    And History is following
        | Id     | Command               | Action | Altered   |
        | 4      |                       |        | 2         |
        | 3      |                       |        | 2         |
        | 2      |                       |        | 2         |
        | 1      |                       |        | 2         |


Scenario: Redo a transaction with a package that is no longer available
   When I execute dnf with args "history redo 1 -x filesystem"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Cannot perform Install action because 'filesystem-3.9-2.fc29.x86_64' matches only excluded packages.
        """


Scenario: Redo a transaction with a package that is no longer available and --skip-unavailable is used
   When I execute dnf with args "history redo 1 -x filesystem --skip-unavailable"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
    And stderr contains lines
        """
        Cannot perform Install action because 'filesystem-3.9-2.fc29.x86_64' matches only excluded packages.
        """


Scenario: Redo a transaction that upgraded a package but the package is downgraded on the system
  Given I use repository "dnf-ci-fedora-updates"
  Given I use repository "dnf-ci-thirdparty"
    And I successfully execute dnf with args "install glibc-2.3.1-10"
    And I successfully execute dnf with args "upgrade glibc-2.28-26.fc29"
    And I successfully execute dnf with args "downgrade glibc-2.28-9.fc29"
    And I successfully execute dnf with args "history redo last-1"
   Then Transaction is following
        | Action        | Package                                    |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64                |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64  |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64         |


Scenario: Redo a transaction that removed a package and the package is was removed from the system already
   When I execute dnf with args "history redo last"
   Then the exit code is 0
    And Transaction is empty


Scenario: Redo a transaction that installed a package and the package is still on the system
  Given I successfully execute dnf with args "install glibc"
   When I execute dnf with args "history redo last"
   Then the exit code is 0
    And Transaction is empty


Scenario: Redo a transaction that installed a package and the package is still on the system, but in a different version
  Given I use repository "dnf-ci-fedora-updates"
    And I successfully execute dnf with args "install glibc-2.28-9.fc29"
    And I successfully execute dnf with args "upgrade glibc-2.28-26.fc29"
   When I execute dnf with args "history redo last-1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | downgrade     | glibc-0:2.28-9.fc29.x86_64                |
        | downgrade     | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
        | downgrade     | glibc-common-0:2.28-9.fc29.x86_64         |
