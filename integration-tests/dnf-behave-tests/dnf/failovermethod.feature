Feature: Test behavior of failovermethod repo config option

@bz2039906
Scenario: No error in stderr when failovermethod=priority is present in repo config
Given I configure a new repository "failovermethod" with
      | key             | value         |
      | failovermethod  | priority      |
 When I execute dnf with args "repo list"
 Then the exit code is 0
  And stderr is empty
