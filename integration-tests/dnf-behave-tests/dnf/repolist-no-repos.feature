Feature: Repo list (alias repolist) when there are no repositories


Scenario: Repolist without arguments
   When I execute dnf with args "repolist"
   Then the exit code is 0
    And stdout is empty
    And stderr is empty


Scenario: Repo list with "--enabled"
   When I execute dnf with args "repo list --enabled"
   Then the exit code is 0
    And stdout is empty
    And stderr is empty


Scenario: Repo list with "--disabled"
   When I execute dnf with args "repo list --disabled"
   Then the exit code is 0
    And stdout is empty
    And stderr is empty


Scenario: Repo list with "--all"
   When I execute dnf with args "repo list --all"
   Then the exit code is 0
    And stdout is empty
    And stderr is empty
