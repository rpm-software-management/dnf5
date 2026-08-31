Feature: Testing environments


Scenario: group installed before environment is not removed with the environment
 Given I use repository "comps-upgrade-1"
   And I successfully execute dnf with args "group install a-group"
   And I successfully execute dnf with args "environment install AB-environment"
  When I execute dnf with args "environment remove AB-environment"
   Then the exit code is 0
    And Transaction is following
        | Action     | Package        |
        | env-remove | AB-environment |


Scenario: group installed after environment is not removed with the environment
 Given I use repository "comps-upgrade-1"
   And I successfully execute dnf with args "environment install AB-environment"
   And I successfully execute dnf with args "group install a-group"
  When I execute dnf with args "environment remove AB-environment"
   Then the exit code is 0
    And Transaction is following
        | Action     | Package        |
        | env-remove | AB-environment |
