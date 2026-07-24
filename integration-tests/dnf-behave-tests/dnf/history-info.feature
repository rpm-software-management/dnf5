Feature: History info command


Background: Set up dnf-ci-fedora repository
  Given I use repository "dnf-ci-fedora"
  And I successfully execute dnf with args "install abcde --comment this_is_a_comment"


@bz1773679
Scenario: history info shows comment to transaction
  When I execute dnf with args "history info"
  Then the exit code is 0
  Then stdout contains "Comment        : this_is_a_comment"


@bz1845800
Scenario: history info for installing a group
  Given I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And dnf5 transaction items for transaction "last" are
        | action  | package                         | reason     | repository        |
        | Install | filesystem-0:3.9-2.fc29.x86_64  | Group      | dnf-ci-fedora     |
        | Install | lame-0:3.100-4.fc29.x86_64      | Group      | dnf-ci-fedora     |
        | Install | setup-0:2.12.1-1.fc29.noarch    | Dependency | dnf-ci-fedora     |
        | Install | lame-libs-0:3.100-4.fc29.x86_64 | Dependency | dnf-ci-fedora     |
        | Install | dnf-ci-testgroup                | User       | dnf-ci-thirdparty |


@bz1845800
Scenario: history info for installing a group when there are upgrades
  Given I successfully execute dnf with args "install lame"
    And I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And dnf5 transaction items for transaction "last" are
        | action         | package                         | reason     | repository            |
        | Install        | filesystem-0:3.9-2.fc29.x86_64  | Group      | dnf-ci-fedora         |
        | Install        | setup-0:2.12.1-1.fc29.noarch    | Dependency | dnf-ci-fedora         |
        | Upgrade        | lame-0:3.100-5.fc29.x86_64      | User       | dnf-ci-fedora-updates |
        | Upgrade        | lame-libs-0:3.100-5.fc29.x86_64 | Dependency | dnf-ci-fedora-updates |
        | Replaced       | lame-0:3.100-4.fc29.x86_64      | User       | @System               |
        | Replaced       | lame-libs-0:3.100-4.fc29.x86_64 | Dependency | @System               |
        | Install        | dnf-ci-testgroup                | User       | dnf-ci-thirdparty     |


Scenario: history info for installing an environment
  Given I use repository "comps-group"
    And I successfully execute dnf with args "install @no-name-env"
   Then dnf5 transaction items for transaction "last" are
        | action  | package                          | reason     | repository  |
        | Install | test-package-0:1.0-1.fc29.noarch | Group      | comps-group |
        | Install | test-group                       | Dependency | comps-group |
        | Install | no-name-env                      | User       | comps-group |
