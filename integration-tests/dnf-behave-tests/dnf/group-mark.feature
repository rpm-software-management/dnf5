Feature: Testing group marking via group install/remove --no-packages

# dnf-ci-testgroup structure:
#   mandatory: filesystem (requires setup)
#   default: lame (requires lame-libs)
#   optional: flac
#   conditional: wget, requires filesystem-content

Scenario: Mark group as installed (using group install --no-packages)
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group list dnf-ci-testgroup"
   Then the exit code is 0
    And stdout contains "dnf-ci-testgroup.*no"
   When I execute dnf with args "group install --no-packages dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | absent        | setup-0:2.12.1-1.fc29.noarch      |
        | absent        | filesystem-0:3.9-2.fc29.x86_64    |
        | absent        | lame-0:3.100-4.fc29.x86_64        |
        | absent        | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-install | DNF-CI-Testgroup                  |
   When I execute dnf with args "group list dnf-ci-testgroup"
   Then the exit code is 0
    And stdout contains "dnf-ci-testgroup.*yes"


Scenario: Unmark group as installed (using group remove --no-packages)
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install --no-packages dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | absent        | setup-0:2.12.1-1.fc29.noarch      |
        | absent        | filesystem-0:3.9-2.fc29.x86_64    |
        | absent        | lame-0:3.100-4.fc29.x86_64        |
        | absent        | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-install | DNF-CI-Testgroup                  |
   When I execute dnf with args "group list dnf-ci-testgroup"
   Then the exit code is 0
    And stdout contains "dnf-ci-testgroup.*yes"
   When I execute dnf with args "group remove --no-packages dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | absent        | setup-0:2.12.1-1.fc29.noarch      |
        | absent        | filesystem-0:3.9-2.fc29.x86_64    |
        | absent        | lame-0:3.100-4.fc29.x86_64        |
        | absent        | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-remove  | DNF-CI-Testgroup                  |
   When I execute dnf with args "group list dnf-ci-testgroup"
   Then the exit code is 0
    And stdout contains "dnf-ci-testgroup.*no"
