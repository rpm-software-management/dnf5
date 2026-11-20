Feature: Tests versionlock with installonly packages


Background: Set up installonly repo and configs
  Given I use repository "installonly"
    And I configure dnf with
        | key               | value              |
        | installonly_limit | 2                  |
        | installonlypkgs   | kernel,kernel-core |


Scenario: Locked installonly kernel is not removed by an upgrade
  Given I successfully execute dnf with args "install kernel-4.18.16 kernel-4.19.15"
    And I successfully execute dnf with args "versionlock installonly kernel*-4.18.16"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | remove        | kernel-0:4.19.15-300.fc29.x86_64      |
        | install       | kernel-core-0:4.20.6-300.fc29.x86_64  |
        | install       | kernel-0:4.20.6-300.fc29.x86_64       |


Scenario: Locked installonly kernel can be removed manually
  Given I successfully execute dnf with args "install kernel-4.18.16 kernel-4.19.15"
    And I successfully execute dnf with args "versionlock installonly kernel*-4.18.16"
   When I execute dnf with args "remove kernel*-4.18.16"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | kernel-core-0:4.18.16-300.fc29.x86_64 |
        | remove        | kernel-0:4.18.16-300.fc29.x86_64      |


Scenario: available installonly package can be excluded by versionlock
  Given I successfully execute dnf with args "install kernel-4.18.16"
    And I successfully execute dnf with args "versionlock exclude kernel*-4.20.6"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.19.15-300.fc29.x86_64 |
        | install       | kernel-0:4.19.15-300.fc29.x86_64      |


Scenario: new installonly version can be installed although older one is locked
  Given I successfully execute dnf with args "install kernel-4.18.16"
    And I successfully execute dnf with args "versionlock installonly kernel*-4.18.16"
   When I execute dnf with args "install kernel-4.19.15"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-0:4.19.15-300.fc29.x86_64      |
        | install-dep   | kernel-core-0:4.19.15-300.fc29.x86_64 |


Scenario: new installonly version can be installed via an update although older one is locked
  Given I successfully execute dnf with args "install kernel-4.18.16"
    And I successfully execute dnf with args "versionlock installonly kernel*-4.18.16"
   When I execute dnf with args "update"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | kernel-core-0:4.20.6-300.fc29.x86_64  |
        | install       | kernel-0:4.20.6-300.fc29.x86_64       |
