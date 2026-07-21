Feature: install with --from-repo


Scenario: Install "abcde" from repository "from-repo", dependecies from enabled repositories
  Given I use repository "from-repo"
    And I use repository "from-repo-updates1"
    And I use repository "from-repo-updates2"
   When I execute dnf with args "install --from-repo=from-repo abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | abcde-0:2.9.2-1.fc29.noarch           |
        | install-dep   | wget-0:1.19.8-2.fc29.x86_64           |
        | install-weak  | flac-0:1.3.3-3.fc29.x86_64            |
    And package state is
        | package                                | reason          | from_repo          |
        | abcde-2.9.2-1.fc29.noarch              | User            | from-repo          |
        | wget-1.19.8-2.fc29.x86_64              | Dependency      | from-repo-updates2 |
        | flac-1.3.3-3.fc29.x86_64               | Weak Dependency | from-repo-updates2 |
    And dnf5 transaction items for transaction "last" are
        | action  | package                          | reason          | repository         |
        | Install | abcde-0:2.9.2-1.fc29.noarch      | User            | from-repo          |
        | Install | wget-0:1.19.8-2.fc29.x86_64      | Dependency      | from-repo-updates2 |
        | Install | flac-0:1.3.3-3.fc29.x86_64       | Weak Dependency | from-repo-updates2 |


Scenario: Install "abcde" and "wget" from repository "from-repo", dependecies from enabled repositories
  Given I use repository "from-repo"
    And I use repository "from-repo-updates1"
    And I use repository "from-repo-updates2"
   When I execute dnf with args "install --from-repo=from-repo abcde wget"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | abcde-0:2.9.2-1.fc29.noarch           |
        | install       | wget-0:1.19.5-5.fc29.x86_64           |
        | install-weak  | flac-0:1.3.3-3.fc29.x86_64            |
    And package state is
        | package                                | reason          | from_repo          |
        | abcde-2.9.2-1.fc29.noarch              | User            | from-repo          |
        | wget-1.19.5-5.fc29.x86_64              | User            | from-repo          |
        | flac-1.3.3-3.fc29.x86_64               | Weak Dependency | from-repo-updates2 |
    And dnf5 transaction items for transaction "last" are
        | action  | package                          | reason          | repository         |
        | Install | abcde-0:2.9.2-1.fc29.noarch      | User            | from-repo          |
        | Install | wget-0:1.19.5-5.fc29.x86_64      | User            | from-repo          |
        | Install | flac-0:1.3.3-3.fc29.x86_64       | Weak Dependency | from-repo-updates2 |
