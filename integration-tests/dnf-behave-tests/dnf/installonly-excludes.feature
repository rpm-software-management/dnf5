Feature: Test installonly packages in combination with excludes


Background:
  Given I use repository "installonly"
    And I configure dnf with
        | key                          | value        |
        | installonlypkgs              | installonlyA |
        | installonly_limit            | 2            |
    And I successfully execute dnf with args "install installonlyA-1.0 installonlyA-2.0"


Scenario: Newest installonly package is skipped if it is excluded
   When I execute dnf with args "upgrade installonlyA -x installonlyA-2.2"
   Then the exit code is 0
    And stdout is
    """
    Nothing to do.
    """
    And stderr is
    """
    <REPOSYNC>
    """


Scenario: Automatic removal skips oldest installonly package if it is excluded
   When I execute dnf with args "upgrade installonlyA -x installonlyA-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                     |
        | install       | installonlyA-0:2.2-1.x86_64 |
        | remove        | installonlyA-0:2.0-1.x86_64 |


Scenario: Installonly update does nothing when all installed packages (full limit) are excluded
   When I execute dnf with args "upgrade installonlyA -x installonlyA-1.0 -x installonlyA-2.0"
   Then the exit code is 0
    And stdout is
    """
    Nothing to do.
    """
    And stderr is
    """
    <REPOSYNC>
    """
