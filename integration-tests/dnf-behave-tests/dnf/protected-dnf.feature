# - doesn't makes sense to test in installroot
# - a potentially destructive test
@no_installroot
Feature: Dnf protects itself


@not.with_os=rhel__lt__11
Scenario: Dnf5 when installed protects itself
  Given I execute dnf with args "install dnf5"
   Then the exit code is 0
   When I execute dnf with args "remove dnf5"
   Then the exit code is 1
    And stderr contains "operation would result in removing the following protected packages: dnf5"
