Feature: Test group remove when repositories are disabled

Background: Install group DNF-CI-Testgroup
    Given I use repository "dnf-ci-thirdparty"
      And I use repository "dnf-ci-thirdparty-2"
      And I use repository "dnf-ci-fedora"
     When I execute dnf with args "group install dnf-ci-testgroup"
     Then the exit code is 0
      And Transaction is following
          | Action        | Package                           |
          | install-group | filesystem-0:3.9-2.fc29.x86_64    |
          | install-group | lame-0:3.100-4.fc29.x86_64        |
          | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
          | install-dep   | lame-libs-0:3.100-4.fc29.x86_64   |
          | group-install | DNF-CI-Testgroup                  |

Scenario: Remove group with disabled repository
   When I execute dnf with args "group remove dnf-ci-testgroup --disable-repo=dnf-ci-thirdparty"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | filesystem-0:3.9-2.fc29.x86_64    |
        | remove        | lame-0:3.100-4.fc29.x86_64        |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch      |
        | remove-unused | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-remove  | DNF-CI-Testgroup                  |

@bz2064341
Scenario: Remove group with no enabed repository
   When I execute dnf with args "group remove dnf-ci-testgroup --disable-repo=\*"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | filesystem-0:3.9-2.fc29.x86_64    |
        | remove        | lame-0:3.100-4.fc29.x86_64        |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch      |
        | remove-unused | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-remove  | DNF-CI-Testgroup                  |
