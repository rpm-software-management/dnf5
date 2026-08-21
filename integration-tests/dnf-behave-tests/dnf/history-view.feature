Feature: Transaction history userinstalled, list and info

Background:
  Given I use repository "dnf-ci-fedora"


Scenario: List userinstalled packages
   When I execute dnf with args "install abcde basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | abcde-0:2.9.2-1.fc29.noarch               |
        | install       | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64               |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-weak  | flac-0:1.3.2-8.fc29.x86_64                |
    And package reasons are
        | Package                      | Reason          |
        | abcde-2.9.2-1.fc29.noarch    | User            |
        | basesystem-11-6.fc29.noarch  | User            |
        | filesystem-3.9-2.fc29.x86_64 | Dependency      |
        | flac-1.3.2-8.fc29.x86_64     | Weak Dependency |
        | setup-2.12.1-1.fc29.noarch   | Dependency      |
        | wget-1.19.5-5.fc29.x86_64    | Dependency      |


Scenario: History info
  Given I successfully execute dnf with args "install abcde"
   When I execute dnf with args "install setup"
    Then dnf5 transaction items for transaction "last" are
        | action  | package                         | reason     | repository        |
        | Install | setup-0:2.12.1-1.fc29.noarch    | User       | dnf-ci-fedora     |
   When I execute dnf with args "remove abcde"
   Then the exit code is 0
    And dnf5 transaction items for transaction "last" are
        | action  | package                         | reason     | repository |
        | Remove  | abcde-0:2.9.2-1.fc29.noarch     | User       | @System    |
        | Remove  | flac-0:1.3.2-8.fc29.x86_64      | Clean      | @System    |
        | Remove  | wget-0:1.19.5-5.fc29.x86_64     | Clean      | @System    |


Scenario: History info in range - transaction merging
  Given I successfully execute dnf with args "install abcde"
  Given I successfully execute dnf with args "remove abcde"
  Given I successfully execute dnf with args "install abcde"
   When I use repository "dnf-ci-fedora-updates"
    And I execute dnf with args "update"
   Then the exit code is 0
    And dnf5 transaction items for transaction "last" are
        | action   | package                         | reason          | repository            |
        | Upgrade  | abcde-0:2.9.3-1.fc29.noarch     | User            | dnf-ci-fedora-updates |
        | Upgrade  | flac-0:1.3.3-3.fc29.x86_64      | Weak Dependency | dnf-ci-fedora-updates |
        | Upgrade  | wget-0:1.19.6-5.fc29.x86_64     | Dependency      | dnf-ci-fedora-updates |
        | Replaced | abcde-0:2.9.2-1.fc29.noarch     | User            | @System               |
        | Replaced | flac-0:1.3.2-8.fc29.x86_64      | Weak Dependency | @System               |
        | Replaced | wget-0:1.19.5-5.fc29.x86_64     | Dependency      | @System               |
    And dnf5 transaction items for transaction "last-1..last" are
        | action   | package                         | reason          | repository            |
        | Upgrade  | abcde-0:2.9.3-1.fc29.noarch     | User            | dnf-ci-fedora-updates |
        | Upgrade  | flac-0:1.3.3-3.fc29.x86_64      | Weak Dependency | dnf-ci-fedora-updates |
        | Upgrade  | wget-0:1.19.6-5.fc29.x86_64     | Dependency      | dnf-ci-fedora-updates |
        | Replaced | abcde-0:2.9.2-1.fc29.noarch     | User            | @System               |
        | Replaced | flac-0:1.3.2-8.fc29.x86_64      | Weak Dependency | @System               |
        | Replaced | wget-0:1.19.5-5.fc29.x86_64     | Dependency      | @System               |
        | Install  | abcde-0:2.9.2-1.fc29.noarch     | User            | dnf-ci-fedora         |
        | Install  | wget-0:1.19.5-5.fc29.x86_64     | Dependency      | dnf-ci-fedora         |
        | Install  | flac-0:1.3.2-8.fc29.x86_64      | Weak Dependency | dnf-ci-fedora         |
    And dnf5 transaction items for transaction "last-2..last" are
        | action   | package                         | reason          | repository            |
        | Upgrade  | abcde-0:2.9.3-1.fc29.noarch     | User            | dnf-ci-fedora-updates |
        | Upgrade  | flac-0:1.3.3-3.fc29.x86_64      | Weak Dependency | dnf-ci-fedora-updates |
        | Upgrade  | wget-0:1.19.6-5.fc29.x86_64     | Dependency      | dnf-ci-fedora-updates |
        | Replaced | abcde-0:2.9.2-1.fc29.noarch     | User            | @System               |
        | Replaced | flac-0:1.3.2-8.fc29.x86_64      | Weak Dependency | @System               |
        | Replaced | wget-0:1.19.5-5.fc29.x86_64     | Dependency      | @System               |
        | Install  | abcde-0:2.9.2-1.fc29.noarch     | User            | dnf-ci-fedora         |
        | Install  | wget-0:1.19.5-5.fc29.x86_64     | Dependency      | dnf-ci-fedora         |
        | Install  | flac-0:1.3.2-8.fc29.x86_64      | Weak Dependency | dnf-ci-fedora         |
        | Remove   | abcde-0:2.9.2-1.fc29.noarch     | User            | @System               |
        | Remove   | flac-0:1.3.2-8.fc29.x86_64      | Clean           | @System               |
        | Remove   | wget-0:1.19.5-5.fc29.x86_64     | Clean           | @System               |
    And dnf5 transaction items for transaction "last-2..last-1" are
        | action   | package                         | reason          | repository            |
        | Install  | abcde-0:2.9.2-1.fc29.noarch     | User            | dnf-ci-fedora         |
        | Install  | wget-0:1.19.5-5.fc29.x86_64     | Dependency      | dnf-ci-fedora         |
        | Install  | flac-0:1.3.2-8.fc29.x86_64      | Weak Dependency | dnf-ci-fedora         |
        | Remove   | abcde-0:2.9.2-1.fc29.noarch     | User            | @System               |
        | Remove   | flac-0:1.3.2-8.fc29.x86_64      | Clean           | @System               |
        | Remove   | wget-0:1.19.5-5.fc29.x86_64     | Clean           | @System               |


Scenario: History info of package
  Given I successfully execute dnf with args "install abcde"
  Given I successfully execute dnf with args "remove abcde"
   Then dnf5 transaction items for transaction "last-1" are
        | action  | package                     | reason          | repository    |
        | Install | abcde-0:2.9.2-1.fc29.noarch | User            | dnf-ci-fedora |
        | Install | wget-0:1.19.5-5.fc29.x86_64 | Dependency      | dnf-ci-fedora |
        | Install | flac-0:1.3.2-8.fc29.x86_64  | Weak Dependency | dnf-ci-fedora |
    And dnf5 transaction items for transaction "last" are
        | action  | package                         | reason     | repository |
        | Remove  | abcde-0:2.9.2-1.fc29.noarch     | User       | @System    |
        | Remove  | flac-0:1.3.2-8.fc29.x86_64      | Clean      | @System    |
        | Remove  | wget-0:1.19.5-5.fc29.x86_64     | Clean      | @System    |


Scenario: history info aaa (nonexistent package)
   When I execute dnf with args "history info --contains-pkgs=aaa"
   Then the exit code is 0
    And stderr is empty
    And stdout is
        """
        No match found, history info defaults to considering only the last transaction, specify "1..last" range to search all transactions.
        """


Scenario: history info aaa (nonexistent package) when other package is present
  Given I successfully execute dnf with args "install abcde"
   When I execute dnf with args "history info --contains-pkgs=aaa"
   Then the exit code is 0
    And stderr is empty
    And stdout is
        """
        No match found, history info defaults to considering only the last transaction, specify "1..last" range to search all transactions.
        """


Scenario: history info aaa (nonexistent package) when full range specified
   When I execute dnf with args "history info 0..last --contains-pkgs=aaa"
   Then the exit code is 0
    And stderr is empty
    And stdout is empty
