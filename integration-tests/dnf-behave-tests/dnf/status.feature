Feature: Testing the status command


Scenario: Status shows summary when no packages are installed
  Given I use repository "comps-group"
   When I execute dnf with args "status"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout contains lines matching
        """
        Groups\s+: 2
        Environments\s+: 2
        Enabled Repositories\s+: 1
        """
    And stdout contains "Packages"
    And stdout contains "Installed Size"
    And stdout does not contain "Last Transaction"


Scenario: Status shows updated counts after installing a group
  Given I use repository "comps-group"
    And I successfully execute dnf with args "group install test-group"
   When I execute dnf with args "status"
   Then the exit code is 0
    And stdout contains lines matching
        """
        Groups\s+: 3
        Environments\s+: 2
        Packages\s+: 3
        Enabled Repositories\s+: 1
        """
    And stdout contains "Installed Size"
    And stdout contains "Last Transaction"
