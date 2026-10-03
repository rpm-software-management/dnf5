Feature: Vendor change control command-line options


Background:
  Given I use repository "vendor-hints"
    And I configure dnf with
        | key                 | value |
        | allow_vendor_change | False |


Scenario: --allow-vendor-change-to allows switching to specific vendor
  # wrench has update from Vendor A to Vendor B. Only the change TO Vendor B is allowed.
  Given I successfully execute dnf with args "install wrench"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "--dump-vendor-policies --allow-vendor-change-to='Vendor B' upgrade"
   Then the exit code is 0
    And stdout contains "======== Vendor Change Policies: ========"
    And stdout contains "Policy #0: source: text:COMMAND LINE\n  in:=\*\"Vendor B\""
    And Transaction is following
        | Action  | Package             |
        | upgrade | wrench-2.0-1.noarch |
    And stdout does not contain "Skipping"


Scenario: --allow-vendor-change-to blocks when vendor does not match
  # wrench has update from Vendor A to Vendor B, but we only allow changes TO Vendor C.
  Given I successfully execute dnf with args "install wrench"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "--allow-vendor-change-to='Vendor C' upgrade"
   Then the exit code is 0
    And Transaction is empty
    And stdout contains "Skipping 1 package due to vendor change restriction"


Scenario: --allow-vendor-change-to supports glob patterns
  # Using glob pattern "Vendor*" to match "Vendor B".
  Given I successfully execute dnf with args "install wrench"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "--allow-vendor-change-to='Vendor*' upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package             |
        | upgrade | wrench-2.0-1.noarch |


Scenario: --allow-vendor-change-from allows switching from specific vendor
  # wrench-2.0 (Vendor B) is installed. Downgrade to wrench-1.0 (Vendor A) is allowed FROM Vendor B.
  Given I use repository "vendor-hints-updates"
    And I successfully execute dnf with args "install wrench-2.0"
    And I drop repository "vendor-hints-updates"
    And I use repository "vendor-hints"
   When I execute dnf with args "--dump-vendor-policies --allow-vendor-change-from='Vendor B' downgrade '*'"
   Then the exit code is 0
    And stdout contains "======== Vendor Change Policies: ========"
    And stdout contains "Policy #0: source: text:COMMAND LINE\n  out:=\*\"Vendor B\""
    And Transaction is following
        | Action    | Package             |
        | downgrade | wrench-1.0-1.noarch |


Scenario: --allow-vendor-change-from blocks when vendor does not match
  # wrench-2.0 (Vendor B) is installed. Downgrade to Vendor A is only allowed FROM Vendor C.
  Given I use repository "vendor-hints-updates"
    And I successfully execute dnf with args "install wrench-2.0"
    And I drop repository "vendor-hints-updates"
    And I use repository "vendor-hints"
   When I execute dnf with args "--allow-vendor-change-from='Vendor C' downgrade '*'"
   Then the exit code is 1
    And Transaction is empty
    And stdout contains "Skipping 1 package due to vendor change restriction"


Scenario: --allow-vendor-change-for-pkgs allows vendor change for specific package
  # wrench and nail both have vendor changes. Only wrench is explicitly allowed.
  Given I successfully execute dnf with args "install wrench nail"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "--dump-vendor-policies --allow-vendor-change-for-pkgs=wrench upgrade"
   Then the exit code is 0
    And stdout contains "======== Vendor Change Policies: ========"
    And stdout contains "Policy #0: source: text:COMMAND LINE\n  @in:\[name=\*\"wrench\"\],out:\[name=\*\"wrench\"\]"
    And Transaction is following
        | Action  | Package             |
        | upgrade | wrench-2.0-1.noarch |
    And stdout contains "Skipping 1 package due to vendor change restriction"


Scenario: --allow-vendor-change-for-pkgs supports comma-separated list
  # Both wrench and nail are allowed via comma-separated list.
  Given I successfully execute dnf with args "install wrench nail"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "--dump-vendor-policies --allow-vendor-change-for-pkgs='wrench,nail' upgrade"
   Then the exit code is 0
    And stdout contains "======== Vendor Change Policies: ========"
    And stdout contains "Policy #0: source: text:COMMAND LINE\n  @in:\[name=\*\"wrench\"\],in:\[name=\*\"nail\"\],out:\[name=\*\"wrench\"\],out:\[name=\*\"nail\"\]"
    And Transaction is following
        | Action  | Package             |
        | upgrade | wrench-2.0-1.noarch |
        | upgrade | nail-2.0-1.noarch   |
    And stdout does not contain "Skipping"


Scenario: --allow-vendor-change-for-pkgs supports glob patterns
  # Using glob pattern to match both wrench and other packages starting with 'w'.
  Given I successfully execute dnf with args "install wrench nail"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "--allow-vendor-change-for-pkgs='w*' upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package             |
        | upgrade | wrench-2.0-1.noarch |
    And stdout contains "Skipping 1 package due to vendor change restriction"


Scenario: --allow-vendor-change-for-pkgs can be specified multiple times
  # Multiple invocations accumulate the package patterns.
  Given I successfully execute dnf with args "install wrench nail"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "--allow-vendor-change-for-pkgs=wrench --allow-vendor-change-for-pkgs=nail upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package             |
        | upgrade | wrench-2.0-1.noarch |
        | upgrade | nail-2.0-1.noarch   |


Scenario: Combining --allow-vendor-change-to and --allow-vendor-change-from
  # Allows change from Vendor A to Vendor B by specifying both directions.
  Given I successfully execute dnf with args "install wrench"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "--dump-vendor-policies --allow-vendor-change-from='Vendor A' --allow-vendor-change-to='Vendor B' upgrade"
   Then the exit code is 0
    And stdout contains "======== Vendor Change Policies: ========"
    And stdout contains "Policy #0: source: text:COMMAND LINE\n  out:=\*\"Vendor A\""
    And stdout contains "Policy #1: source: text:COMMAND LINE\n  in:=\*\"Vendor B\""
    And Transaction is following
        | Action  | Package             |
        | upgrade | wrench-2.0-1.noarch |


Scenario: Multiple --allow-vendor-change-to options accumulate
  # Multiple vendor patterns are accumulated.
  Given I successfully execute dnf with args "install wrench nail"
    And I use repository "vendor-hints-updates"
   When I execute dnf with args "--dump-vendor-policies --allow-vendor-change-to='Vendor B' --allow-vendor-change-to='Vendor C' upgrade"
   Then the exit code is 0
    And stdout contains "======== Vendor Change Policies: ========"
    And stdout contains "Policy #0: source: text:COMMAND LINE\n  in:=\*\"Vendor B\""
    And stdout contains "Policy #1: source: text:COMMAND LINE\n  in:=\*\"Vendor C\""
    And Transaction is following
        | Action  | Package             |
        | upgrade | wrench-2.0-1.noarch |
        | upgrade | nail-2.0-1.noarch   |
