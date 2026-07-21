Feature: Testing localpkg_gpgcheck


Scenario: locally installing unsigned pkg is allowed (localpkg_gpgcheck is off by defaul)
   When I execute dnf with args "install {context.dnf.fixturesdir}/repos/unsigned/x86_64/sarcina-1.0-1.fc29.x86_64.rpm"
   Then the exit code is 0
    And DNF Transaction is following
        | Action        | Package                     |
        | install       | sarcina-0:1.0-1.fc29.x86_64 |


Scenario: locally installing unsigned pkg is not allowed when localpkg_gpgcheck is on
  Given I configure dnf with
        | key               | value |
        | localpkg_gpgcheck | 1     |
   When I execute dnf with args "install {context.dnf.fixturesdir}/repos/unsigned/x86_64/sarcina-1.0-1.fc29.x86_64.rpm"
   Then the exit code is 1
    And stderr contains "Signature verification failed."
