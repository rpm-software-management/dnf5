Feature: Test installonly packages locked against automatic removal


Background:
  Given I use repository "installonly"
    And I configure dnf with
        | key                          | value        |
        | installonlypkgs              | installonlyA |
        | installonly_limit            | 2            |
    And I successfully execute dnf with args "install installonlyA-1.0 installonlyA-2.0"


Scenario: Locked installonly package is not removed when limit is exceeded
  Given I successfully execute dnf with args "installonly-lock add installonlyA-1.0"
   When I execute dnf with args "install installonlyA-2.2"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                     |
        | install       | installonlyA-0:2.2-1.x86_64 |
        | remove        | installonlyA-0:2.0-1.x86_64 |
        | unchanged     | installonlyA-0:1.0-1.x86_64 |


Scenario: Unlocked installonly package can be removed again by the limit
  Given I successfully execute dnf with args "installonly-lock add installonlyA-1.0"
    And I successfully execute dnf with args "installonly-lock delete installonlyA-1.0"
   When I execute dnf with args "install installonlyA-2.2"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                     |
        | install       | installonlyA-0:2.2-1.x86_64 |
        | unchanged     | installonlyA-0:2.0-1.x86_64 |
        | remove        | installonlyA-0:1.0-1.x86_64 |


Scenario: List shows locked installonly packages
  Given I successfully execute dnf with args "installonly-lock add installonlyA-1.0"
   When I execute dnf with args "installonly-lock list"
   Then the exit code is 0
    And stdout is
    """
    installonlyA-0:1.0-1.x86_64
    """
