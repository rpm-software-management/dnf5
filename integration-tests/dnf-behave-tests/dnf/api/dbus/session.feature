@dnf5daemon
@not.with_mode=dnf5
Feature: D-Bus api: session manager

Scenario: Open and close a session
  Given I open a dnf5daemon session
   Then the dnf5daemon session object path matches "^/org/rpm/dnf/v0/[0-9a-f]{32}$"
   When I close the dnf5daemon session
   Then the dnf5daemon session close returns True
   When I close the dnf5daemon session
    And the dnf5daemon session close returns False
