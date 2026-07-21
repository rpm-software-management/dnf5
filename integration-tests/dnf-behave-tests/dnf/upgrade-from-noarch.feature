@dnf5daemon
Feature: Upgrade package from noarch to arch

Background: Install package to upgrade
   Given I use repository "upgrade-from-noarch"
    When I execute dnf with args "install it-1.0"
    Then the exit code is 0
     And DNF Transaction is following
         | Action        | Package         |
         | install       | it-1.0-1.x86_64 |

Scenario: Upgrade to rpm from file which is not noarch
    When I execute dnf with args "upgrade {context.dnf.fixturesdir}/repos/upgrade-from-noarch/noarch/it-2.0-1.noarch.rpm"
    Then the exit code is 0
     And DNF Transaction is following
         | Action        | Package         |
         | upgrade       | it-2.0-1.noarch |
