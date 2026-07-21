Feature: Alternative packages are suggested when package is not available


@bz1625586
Scenario: Alternative packages are suggested during package install
Given I use repository "dnf-ci-thirdparty"
 When I execute dnf with args "install IHaveAlternatives"
 Then the exit code is 1
  And stderr contains "Failed to resolve the transaction:"
  And stderr contains "No match for argument: IHaveAlternatives"
  And stderr contains "There are following alternatives for 'IHaveAlternatives': alternator, alternator-alternator-cz"
