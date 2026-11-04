Feature: dnf-automatic loads drop-in configuration files


Scenario: drop-in config files from /usr/share drop-in directory are loaded
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf" with
    """
    [commands]
    download_updates = no
    apply_updates = no

    [emitters]
    emit_via = stdio
    emit_no_updates = yes
    """
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf.d/10-test.conf" with
    """
    [commands]
    download_updates = yes
    """
   When I execute dnf with args "automatic --timer"
   Then the exit code is 0
    And stdout contains "No new upgrades available"


Scenario: drop-in config files from /etc/dnf drop-in directory override system defaults
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf" with
    """
    [commands]
    apply_updates = no

    [emitters]
    emit_via = stdio
    emit_no_updates = no
    """
  Given I create file "/etc/dnf/automatic.conf.d/10-local.conf" with
    """
    [emitters]
    emit_no_updates = yes
    """
   When I execute dnf with args "automatic --timer"
   Then the exit code is 0
    And stdout contains "No new upgrades available"


Scenario: drop-in config files are processed in alphabetical order
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf" with
    """
    [emitters]
    emit_via = stdio
    emit_no_updates = no
    """
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf.d/20-second.conf" with
    """
    [emitters]
    emit_no_updates = yes
    """
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf.d/10-first.conf" with
    """
    [commands]
    download_updates = no
    apply_updates = no
    """
   When I execute dnf with args "automatic --timer"
   Then the exit code is 0
    And stdout contains "No new upgrades available"


Scenario: later drop-in files override earlier values within same directory
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf.d/10-first.conf" with
    """
    [emitters]
    emit_no_updates = no
    """
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf.d/20-second.conf" with
    """
    [emitters]
    emit_no_updates = yes
    emit_via = stdio
    """
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf.d/30-third.conf" with
    """
    [commands]
    download_updates = no
    apply_updates = no
    """
   When I execute dnf with args "automatic --timer"
   Then the exit code is 0
    And stdout contains "No new upgrades available"


Scenario: /etc/dnf/automatic.conf.d overrides both main configs
  Given I create file "/usr/share/dnf5/dnf5-plugins/automatic.conf" with
    """
    [commands]
    apply_updates = no

    [emitters]
    emit_via = stdio
    emit_no_updates = no
    """
  Given I create file "/etc/dnf/automatic.conf.d/10-local.conf" with
    """
    [emitters]
    emit_no_updates = yes
    """
   When I execute dnf with args "automatic --timer"
   Then the exit code is 0
    And stdout contains "No new upgrades available"
