Feature: History of update

Background:
  Given I use repository "dnf-ci-fedora"

Scenario: History of update packages
   # `install setup` step added so that `install abcde` was not the first
   # transaction in history. Dnf due to some error is not able to
   # rollback the very first transaction.
   When I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-1.fc29.noarch              |
   Then the exit code is 0
   When I execute dnf with args "install abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | abcde-0:2.9.2-1.fc29.noarch               |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64               |
        | install-weak  | flac-0:1.3.2-8.fc29.x86_64                |
   When I use repository "dnf-ci-fedora-updates"
    And I execute dnf with args "update"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64               |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
    And History is following
        | Id     | Command               | Action | Altered   |
        | 3      | update                |        | 6         |
        | 2      |                       |        | 3         |
        | 1      |                       |        | 1         |


@bz1612885
Scenario: Rollback update
  Given I successfully execute dnf with args "install setup"
    And I execute dnf with args "install abcde"
    And I use repository "dnf-ci-fedora-updates"
    And I successfully execute dnf with args "update"
   When I execute dnf with args "history rollback last-1"
   Then the exit code is 0
   Then stderr does not contain "Traceback"
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | abcde-0:2.9.2-1.fc29.noarch               |
        | downgrade     | flac-0:1.3.2-8.fc29.x86_64                |
        | downgrade     | wget-0:1.19.5-5.fc29.x86_64               |
   When I execute dnf with args "history rollback last-3"
   Then the exit code is 0
   Then stderr does not contain "Traceback"
    And Transaction is following
        | Action        | Package                                   |
        | remove        | abcde-0:2.9.2-1.fc29.noarch               |
        | remove-unused | flac-0:1.3.2-8.fc29.x86_64                |
        | remove-unused | wget-0:1.19.5-5.fc29.x86_64               |
