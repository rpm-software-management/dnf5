Feature: Check when there is no problem


Scenario Outline: Check <option>
   When I execute dnf with args "check <option>"
   Then the exit code is 0
    And stdout is empty
    And stderr is empty

Examples:
        | option             |
        # no option defaults to "all"
        |                    |
        | --dependencies     |
        | --duplicates       |
        | --obsoleted        |
