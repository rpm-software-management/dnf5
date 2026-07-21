# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Enabling module streams that are not latest


Background:
  Given I use repository "module-problem-latest"


Scenario: Enable a module stream that is not latest fails with --best
   When I execute dnf with args "module enable berries:main --best"
   Then the exit code is 1
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Modular dependency problems with the latest modules:
        Problem: cannot install the best candidate for the job
          - nothing provides module(gooseberry:5.5) needed by module berries:main:5:6c81f848.x86_64
        """


Scenario: Enable a module stream that is not latest doesn't fail with --nobest
   When I execute dnf with args "module enable berries:main --nobest"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-enable     | berries:main       |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | berries   | enabled   | main      |           |
