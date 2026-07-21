Feature: Handling local base url in repository in installroot


Scenario: Handling remote base url in repository in installroot
  Given I use repository "dnf-ci-fedora" as http
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | filesystem-0:3.9-2.fc29.x86_64    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
