Feature: Install RPMs with forcearch

Background: Use dnf-ci-fedora repository
  Given I use repository "dnf-ci-fedora"


# https://github.com/rpm-software-management/dnf5/issues/855
Scenario: Install an RPM with both --releasever and --forcearch given
   When I execute dnf with args "install --forcearch=x86_64 --releasever=29 filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


Scenario: Report error when unsupported architecture is requested
    When I execute dnf with args "install --forcearch=UNKNOWN filesystem"
    Then the exit code is 2
     And stderr matches line by line
     """
     Unsupported architecture "UNKNOWN". Please choose one from.*
     """
