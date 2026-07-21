Feature: Test upgrading and downgrading to package with different epoch


Scenario: Upgrade to RPM with same NVR but different epoch
  Given I use repository "epoch0"
    And I successfully execute dnf with args "install dummy"
    And I use repository "epoch1"
   When I execute dnf with args "upgrade dummy"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                |
        | upgrade       | dummy-1:1.0-1.x86_64   |


Scenario: Upgrade to RPM with same NVR but different epoch from RPM with no epoch
  Given I use repository "epoch"
    And I successfully execute dnf with args "install dummy"
    And I use repository "epoch1"
   When I execute dnf with args "upgrade dummy"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                |
        | upgrade       | dummy-1:1.0-1.x86_64   |


Scenario: Downgrade to RPM with same NVR but different epoch
  Given I use repository "epoch1"
    And I successfully execute dnf with args "install dummy"
    And I use repository "epoch0"
   When I execute dnf with args "downgrade dummy"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                |
        | downgrade     | dummy-0:1.0-1.x86_64   |


@bz1845069
Scenario: Downgrade to RPM with same NVR but no epoch
  Given I use repository "epoch1"
    And I successfully execute dnf with args "install dummy"
    And I use repository "epoch"
   When I execute dnf with args "downgrade dummy"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                |
        | downgrade     | dummy-0:1.0-1.x86_64   |
