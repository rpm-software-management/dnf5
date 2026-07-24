Feature: Test security options for update


Background: Use repository with advisories
  Given I use repository "dnf-ci-security"
   When I execute dnf with args "install bugfix_A-1.0-1 bugfix_B-1.0-1 bugfix_C-1.0-1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | install       | bugfix_A-0:1.0-1.x86_64   |
        | install       | bugfix_B-0:1.0-1.x86_64   |
        | install       | bugfix_C-0:1.0-1.x86_64   |


Scenario: Test security option --bugfix for update
   When I execute dnf with args "upgrade --bugfix"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | bugfix_B-0:1.0-2.x86_64   |
        | upgrade       | bugfix_C-0:1.0-4.x86_64   |


Scenario: Test security option --bugfix for upgrade-minimal
   When I execute dnf with args "upgrade-minimal --bugfix"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | bugfix_B-0:1.0-2.x86_64   |
        | upgrade       | bugfix_C-0:1.0-3.x86_64   |
