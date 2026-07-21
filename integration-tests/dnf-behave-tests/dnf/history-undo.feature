Feature: Transaction history undo


Background:
  Given I use repository "dnf-ci-fedora"


Scenario: Undoing transactions
  Given I successfully execute dnf with args "install filesystem"
   Then History is following
        | Id     | Command               | Action        | Altered   |
        | 1      | install filesystem    |               | 2         |
   When I execute dnf with args "history undo last"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch               |
        | remove        | filesystem-0:3.9-2.fc29.x86_64             |
    And History is following
        | Id     | Command               | Action        | Altered   |
        | 2      | undo last             |               | 2         |
        | 1      | install filesystem    |               | 2         |
   When I execute dnf with args "history undo last"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch               |
        | install       | filesystem-0:3.9-2.fc29.x86_64             |
    And History is following
        | Id     | Command               | Action        | Altered   |
        | 3      | undo last             |               | 2         |
        | 2      | undo last             |               | 2         |
        | 1      | install filesystem    |               | 2         |
   When I execute dnf with args "history undo last-2"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                    |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch               |
        | remove        | filesystem-0:3.9-2.fc29.x86_64             |
    And History is following
        | Id     | Command               | Action        | Altered   |
        | 4      | undo last-2           |               | 2         |
        | 3      | undo last             |               | 2         |
        | 2      | undo last             |               | 2         |
        | 1      | install filesystem    |               | 2         |


@1627111
Scenario: Handle missing packages required for undoing the transaction
    When I execute dnf with args "install wget flac"
    Then the exit code is 0
     And Transaction is following
         | Action        | Package                      |
         | install       | wget-0:1.19.5-5.fc29.x86_64  |
         | install       | flac-0:1.3.2-8.fc29.x86_64   |
   When I drop repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   Then I execute dnf with args "update"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                      |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64   |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64  |
    And History is following
        | Id     | Command               | Action | Altered   |
        | 2      | update                |        | 4         |
        | 1      | install wget flac     |        | 2         |
     Then I execute dnf with args "history undo 2"
     Then the exit code is 1
     And Transaction is empty
     And stderr is
         """
         <REPOSYNC>
         Failed to resolve the transaction:
         Cannot perform Install action, no match for: wget-1.19.5-5.fc29.x86_64.
         Cannot perform Install action, no match for: flac-1.3.2-8.fc29.x86_64.
         You can try to add to command line:
           --skip-unavailable to skip unavailable packages
         """


Scenario: Missing packages are skipped if --skip-unavailable is specified
    When I execute dnf with args "install wget flac"
    Then the exit code is 0
     And Transaction is following
         | Action        | Package                      |
         | install       | wget-0:1.19.5-5.fc29.x86_64  |
         | install       | flac-0:1.3.2-8.fc29.x86_64   |
   When I drop repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   Then I execute dnf with args "update"
    Then the exit code is 0
     And Transaction is following
         | Action        | Package                      |
         | upgrade       | flac-0:1.3.3-3.fc29.x86_64   |
         | upgrade       | wget-0:1.19.6-5.fc29.x86_64  |
     Then I execute dnf with args "history undo last --skip-unavailable"
     Then the exit code is 0
     And Transaction is empty
     And stderr is
         """
         <REPOSYNC>
         Cannot perform Install action, no match for: wget-1.19.5-5.fc29.x86_64.
         Cannot perform Install action, no match for: flac-1.3.2-8.fc29.x86_64.
         """


Scenario: Undo a transaction with a package that is no longer available
  Given I successfully execute dnf with args "install filesystem"
   When I execute dnf with args "history undo 1 -x filesystem"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Cannot perform Remove action because 'filesystem-3.9-2.fc29.x86_64' matches only excluded packages.
        Problem: installed package filesystem-3.9-2.fc29.x86_64 requires setup, but none of the providers can be installed
          - conflicting requests
          - problem with installed package
        """


@bz2010259
@bz2053014
Scenario: Undoing a transaction with Reason Change
  Given I successfully execute dnf with args "install filesystem"
   Then History is following
        | Id     | Command               | Action        | Altered   |
        | 1      | install filesystem    |               | 2         |
    And package reasons are
        | Package                      | Reason          |
        | filesystem-3.9-2.fc29.x86_64 | User            |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |
   When I execute dnf with args "mark weak filesystem"
   Then the exit code is 0
   Then History is following
        | Id     | Command              | Action | Altered   |
        | 2      | mark weak filesystem |        | 1         |
        | 1      | install filesystem   |        | 2         |
    And package reasons are
        | Package                      | Reason          |
        | filesystem-3.9-2.fc29.x86_64 | Weak Dependency |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |
   When I execute dnf with args "history undo last"
   Then the exit code is 0
    And History is following
        | Id     | Command              | Action | Altered   |
        | 3      | history undo last    |        | 1         |
        | 2      | mark weak filesystem |        | 1         |
        | 1      | install filesystem   |        | 2         |
    And package reasons are
        | Package                      | Reason          |
        | filesystem-3.9-2.fc29.x86_64 | User            |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |


@bz2010259
@bz2053014
Scenario: Undoing an older transaction with Reason Change
  Given I successfully execute dnf with args "install filesystem"
   Then History is following
        | Id     | Command               | Action        | Altered   |
        | 1      | install filesystem    |               | 2         |
    And package reasons are
        | Package                      | Reason          |
        | filesystem-3.9-2.fc29.x86_64 | User            |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |
   When I execute dnf with args "mark weak filesystem"
   Then History is following
        | Id     | Command               | Action        | Altered   |
        | 2      | mark weak filesystem  |               | 1         |
        | 1      | install filesystem    |               | 2         |
    And package reasons are
        | Package                      | Reason          |
        | filesystem-3.9-2.fc29.x86_64 | Weak Dependency |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |
   When I execute dnf with args "install wget"
   Then the exit code is 0
   When I execute dnf with args "history undo last-1"
   Then the exit code is 0
    And History is following
        | Id     | Command               | Action | Altered   |
        | 4      | history undo last-1   |        | 1         |
        | 3      | install wget          |        | 1         |
        | 2      | mark weak filesystem  |        | 1         |
        | 1      | install filesystem    |        | 2         |
    And package reasons are
        | Package                      | Reason          |
        | filesystem-3.9-2.fc29.x86_64 | User            |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |
        | wget-1.19.5-5.fc29.x86_64    | User            |


@bz2010259
@bz2053014
Scenario: Undoing a transaction range with Reason Change
  Given I successfully execute dnf with args "install filesystem"
   Then History is following
        | Id     | Command               | Action        | Altered   |
        | 1      | install filesystem    |               | 2         |
    And package reasons are
        | Package                      | Reason          |
        | filesystem-3.9-2.fc29.x86_64 | User            |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |
   When I execute dnf with args "mark weak filesystem"
   Then History is following
        | Id     | Command               | Action        | Altered   |
        | 2      | mark weak filesystem  |               | 1         |
        | 1      | install filesystem    |               | 2         |
    And package reasons are
        | Package                      | Reason          |
        | filesystem-3.9-2.fc29.x86_64 | Weak Dependency |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |
   When I execute dnf with args "install wget"
   Then the exit code is 0
   When I execute dnf with args "history rollback 1"
   Then the exit code is 0
    And History is following
        | Id     | Command              | Action | Altered   |
        | 4      | history rollback 1   |        | 2         |
        | 3      | install wget         |        | 1         |
        | 2      | mark weak filesystem |        | 1         |
        | 1      | install filesystem   |        | 2         |
    And package reasons are
        | Package                      | Reason          |
        | filesystem-3.9-2.fc29.x86_64 | User            |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |


Scenario: Undo a downgrade transaction
  Given I use repository "dnf-ci-fedora-updates"
    And I successfully execute dnf with args "install wget"
    And I successfully execute dnf with args "downgrade wget"
   When I execute dnf with args "history undo last"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                     |
        | upgrade | wget-0:1.19.6-5.fc29.x86_64 |


Scenario: Undo an upgrade transaction with --skip-unavailable where the orignal package is not available
  Given I successfully execute dnf with args "install wget"
    And I use repository "dnf-ci-fedora-updates"
    And I successfully execute dnf with args "upgrade wget"
    And I drop repository "dnf-ci-fedora"
   When I execute dnf with args "history undo last --skip-unavailable"
   Then the exit code is 0
    And Transaction is empty
    And stdout is
        """
        Nothing to do.
        """
    And stderr is
        """
        <REPOSYNC>
        Cannot perform Install action, no match for: wget-1.19.5-5.fc29.x86_64.
        """
