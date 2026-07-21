Feature: Vendor change restriction applies to installonly packages

Background:
  Given I use repository "vendor-hints"
    And I configure dnf with
        | key                 | value |
        | allow_vendor_change | False |
        | installonlypkgs     | kernel |


Scenario: Both regular and installonly packages from different vendor are reported as skipped
  # During dnf upgrade the vendor change callback fires for both wrench-2.0 (a
  # regular package) and kernel-2.0 (an installonly package) from Vendor B.
  # Both are blocked and reported together, exercising the plural output form.
  Given I successfully execute dnf with args "install wrench kernel"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is empty
    And stdout contains lines
        """
        Skipping 2 packages due to vendor change restriction: "Vendor A" -> "Vendor B".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: Installing a specific installonly version from a different vendor shows no skipping section
  # Installing a specific package version (dnf install kernel-2.0) installs it
  # alongside the existing version without replacing it. Because no replacement
  # occurs, the vendor change callback does not fire and the package installs
  # normally regardless of the vendor.
  Given I successfully execute dnf with args "install kernel"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "install kernel-2.0"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package             |
        | install   | kernel-2.0-1.noarch |
        | unchanged | kernel-1.0-1.noarch |
    And stdout does not contain "Skipping"


Scenario: Skipping section shown when installonly upgrade is blocked by vendor change
  # Unlike the general upgrade sweep, "dnf upgrade kernel" explicitly requests an
  # upgrade and the vendor change callback fires. The transaction is empty and
  # the skipping section appears.
  Given I successfully execute dnf with args "install kernel"
    And I use repository "vendor-hints-installonly-upgrade-blocked"
   When I execute dnf with args "upgrade kernel"
   Then the exit code is 0
    And Transaction is empty
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "Vendor A" -> "Vendor B".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: Skipping section shown when installonly versions from different vendors are present
  # kernel-1.0 (Vendor A) is installed. kernel-2.0 (Vendor B) is then installed
  # alongside it (no replacement, so vendor change does not apply).
  # With both versions present, upgrading to kernel-3.0 (Vendor C) triggers the
  # vendor change callback for each installed version. The candidate is blocked and
  # reported with one of the installed vendors in the transition.
  Given I successfully execute dnf with args "install kernel"
    And I use repository "vendor-hints-updates"
    And I successfully execute dnf with args "install kernel-2.0"
    And I use repository "vendor-hints-installonly-multi-vendor"
   When I execute dnf with args "upgrade kernel"
   Then the exit code is 0
    And Transaction is empty
    And stdout contains "Skipping 1 package due to vendor change restriction"
    And stdout contains "-> \"Vendor C\"."
    And stderr contains "--allow-vendor-change to allow changing package vendors"
