Feature: Tests for install with dependencies


@bz1774617
Scenario: Best candidates have conflicting dependencies
  Given I use repository "install-dependencies"
   When I execute dnf with args "install foo bar --no-best"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | foo-0:1.0-1.fc29.x86_64           |
        | install       | bar-0:1.0-1.fc29.x86_64           |
        | install-dep   | lib-0:1.0-1.fc29.x86_64           |
        | conflict      | lib-0:2.0-1.fc29.x86_64           |
        | broken        | foo-0:2.0-1.fc29.x86_64           |
    And stderr contains lines
    """
    Problem: cannot install both lib-2.0-1.fc29.x86_64 from install-dependencies and lib-1.0-1.fc29.x86_64 from install-dependencies
      - package foo-2.0-1.fc29.x86_64 from install-dependencies requires lib-2.0, but none of the providers can be installed
      - package bar-1.0-1.fc29.x86_64 from install-dependencies requires lib-1.0, but none of the providers can be installed
      - cannot install the best candidate for the job
      - conflicting requests
    """
