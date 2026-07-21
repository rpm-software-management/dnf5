@dnf5daemon
Feature: Remove RPMs by provides


Background: Install two providers of foo - foo and bar
  Given I use repository "dnf-ci-provides-alternatives"
   When I execute dnf with args "install foo bar"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | foo-0:1.0-1.noarch              |
        | install       | bar-0:1.0-1.noarch              |



Scenario: Remove an RPM by name and not by provide
   When I execute dnf with args "remove 'foo'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                     |
        | remove        | foo-0:1.0-1.noarch          |
   When I execute dnf with args "remove 'foo'"
   Then the exit code is 0
    And Transaction is empty
