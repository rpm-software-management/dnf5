Feature: Vendor change restriction hints when allow_vendor_change=false

Background:
  Given I use repository "vendor-hints"
    And I configure dnf with
        | key                 | value |
        | allow_vendor_change | False |


Scenario: No skipping section when allow_vendor_change is enabled via command line
  # Verifies that --allow-vendor-change (the hint shown to the user)
  # allows wrench to upgrade to the Vendor B version without a skipping section.
  Given I successfully execute dnf with args "install hammer wrench"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "upgrade --allow-vendor-change"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package             |
        | upgrade | hammer-2.0-1.noarch |
        | upgrade | wrench-2.0-1.noarch |
    And stdout does not contain "Skipping"


Scenario: Skipping section shown alongside a successful partial upgrade
  # hammer has a same-vendor update (Vendor A -> Vendor A) and upgrades normally.
  # wrench has a different-vendor update (Vendor A -> Vendor B) and is skipped.
  Given I successfully execute dnf with args "install hammer wrench"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package              |
        | upgrade | hammer-2.0-1.noarch  |
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "Vendor A" -> "Vendor B".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: Skipping section and hint shown when all upgrades are blocked (nothing to do)
  # Only a different-vendor update for wrench is available; no same-vendor updates exist.
  # The transaction is empty but the skipping section and the hint still appear.
  Given I successfully execute dnf with args "install wrench"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "upgrade wrench"
   Then the exit code is 0
    And Transaction is empty
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "Vendor A" -> "Vendor B".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: Skipping section shown when reinstall is blocked by vendor change
  # chisel-1.0-1.noarch is installed from Vendor A. The only available copy in the
  # enabled repo is from Vendor B (same NEVRA, different vendor). The reinstall
  # fails with a solver error, but the skipping section still appears.
  # vendor-hints is dropped so the solver cannot fall back to the same-vendor copy.
  Given I successfully execute dnf with args "install chisel"
    And I drop repository "vendor-hints"
    And I use repository "vendor-hints-reinstall"
   When I execute dnf with args "reinstall chisel"
   Then the exit code is 1
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "Vendor A" -> "Vendor B".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: Skipping section shown when downgrade is blocked by vendor change
  # wrench-2.0 (Vendor B) is installed as a new package (no vendor check for
  # first-time installs). The base repo provides wrench-1.0 (Vendor A) as the
  # only downgrade candidate. The downgrade is blocked due to vendor change.
  Given I use repository "vendor-hints-updates"
    And I successfully execute dnf with args "install wrench-2.0"
   When I execute dnf with args "downgrade wrench"
   Then the exit code is 1
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "Vendor B" -> "Vendor A".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: Skipping section shown when install-via-obsolete is blocked by vendor change
  # hammer-1.0 (Vendor A) is installed. socket-1.0 (Vendor B) obsoletes hammer, so
  # installing socket would replace hammer with a vendor change. The install fails
  # with a solver error but the skipping section appears for socket.
  # This exercises the SOLVER_TRANSACTION_OBSOLETES path in the detection code.
  Given I successfully execute dnf with args "install hammer"
    And I use repository "vendor-hints-obsolete"
   When I execute dnf with args "install socket"
   Then the exit code is 1
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "Vendor A" -> "Vendor B".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: distro-sync shows skipping section when vendor change is blocked
  # Same code path as upgrade but exercising the distro-sync command.
  Given I successfully execute dnf with args "install hammer wrench"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "distro-sync"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package             |
        | upgrade | hammer-2.0-1.noarch |
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "Vendor A" -> "Vendor B".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: Multiple vendor transitions shown as separate lines in the skipping section
  # wrench-2.0 (Vendor B) and nail-2.0 (Vendor C) are both blocked, producing two
  # distinct vendor transitions in the output.
  Given I successfully execute dnf with args "install hammer wrench nail"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package             |
        | upgrade | hammer-2.0-1.noarch |
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "Vendor A" -> "Vendor B".
        """
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "Vendor A" -> "Vendor C".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: Skipping section renders "(none)" for packages with no vendor metadata
  # bolt-1.0 is installed with no Vendor field. bolt-2.0 (Vendor B) is the only
  # available update. The vendor change from "(none)" to "Vendor B" is blocked and
  # reported with the "(none)" placeholder.
  Given I successfully execute dnf with args "install bolt"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "upgrade bolt"
   Then the exit code is 0
    And Transaction is empty
    And stdout contains lines
        """
        Skipping 1 package due to vendor change restriction: "(none)" -> "Vendor B".
        """
    And stderr contains "--allow-vendor-change to allow changing package vendors"
