@dnf5daemon
Feature: Test upgrading installonly packages - "installonlypkgs" configuration option


Background: Install one version of each test package
  Given I use repository "installonly"
   When I execute dnf with args "install installonlyA-1.0 installonlyB-1.0 kernel-core-4.18.16"
   Then the exit code is 0
     And Transaction is following
        | Action        | Package                               |
        | install       | installonlyA-0:1.0-1.x86_64           |
        | install       | installonlyB-0:1.0-1.x86_64           |
        | install       | kernel-core-0:4.18.16-300.fc29.x86_64 |


Scenario: kernel-core is installonly package by default, and installonlyA is not
   When I execute dnf with args "install installonlyA-2.0 kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | upgrade       | installonlyA-0:2.0-1.x86_64           |
        | install       | kernel-core-0:4.20.6-300.fc29.x86_64  |


Scenario: Add package installonlyA to the installonly packages, kernel-core is installonly by default
   Given I configure dnf with
        | key                          | value         |
        | installonlypkgs              | installonlyA  |
   When I execute dnf with args "install installonlyA-2.0 kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | installonlyA-0:2.0-1.x86_64           |
        | install       | kernel-core-0:4.20.6-300.fc29.x86_64  |


Scenario: Add package installonlyA to the installonly packages, install multiple versions of installonlyA with a single command
   Given I configure dnf with
        | key                          | value         |
        | installonlypkgs              | installonlyA  |
   When I execute dnf with args "install installonlyA-2.0 installonlyA-2.2 installonlyB kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | installonlyA-0:2.0-1.x86_64           |
        | install       | installonlyA-0:2.2-1.x86_64           |
        | upgrade       | installonlyB-0:2.2-1.x86_64           |
        | install       | kernel-core-0:4.20.6-300.fc29.x86_64  |


Scenario: Clear the list of installonly packages and set the package installonlyA to it
   Given I configure dnf with
        | key                          | value         |
        | installonlypkgs              | ,installonlyA |
   When I execute dnf with args "install installonlyA-2.0 installonlyB-2.0 kernel-core"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | installonlyA-0:2.0-1.x86_64           |
        | upgrade       | installonlyB-0:2.0-1.x86_64           |
        | upgrade       | kernel-core-0:4.20.6-300.fc29.x86_64  |


Scenario: Upgrade doesn't install older installonly pkg when never version is already installed
   Given I configure dnf with
        | key                          | value         |
        | installonlypkgs              | installonlyA  |
   When I execute dnf with args "install installonlyA-2.2"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                      |
        | install       | installonlyA-0:2.2-1.x86_64  |
   When I execute dnf with args "upgrade installonlyA-2.0"
   Then the exit code is 0
    And Transaction is empty
