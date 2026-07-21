Feature: dnf-automatic does not emit when no updates are available


Scenario: dnf-automatic does not emit when no updates are available
   When I execute dnf with args "automatic --installupdates"
   Then the exit code is 0
    And Transaction is empty
    And stdout is empty


Scenario: dnf-automatic emits message when no updates are available when emit_no_updates is on
  Given I create file "/etc/dnf/automatic.conf" with
    """
    [emitters]
    emit_no_updates = yes
    """
   When I execute dnf with args "automatic --installupdates"
   Then the exit code is 0
    And Transaction is empty
    And stdout contains "No new upgrades available."
