Feature: DNF allow_vendor_change defaults to false
Background:
  Given I use repository "dnf-ci-vendor-1"
   And I configure dnf with
       | key                 | value |
       | allow_vendor_change | False |
   And I successfully execute dnf with args "install vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | install | vendor-1.0-1.x86_64 |

  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: First Vendor"


Scenario: Upgrade sticks to vendor by default
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.1-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: First Vendor"


Scenario: No upgrade if same vendor not found by default
  Given I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is empty
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: First Vendor"


Scenario: Hint shown when vendor change blocked by default
  Given I use repository "dnf-ci-vendor-1-updates"
   When I execute dnf with args "upgrade vendor"
  Given I drop repository "dnf-ci-vendor-1"
  Given I use repository "dnf-ci-vendor-2"
   When I execute dnf with args "downgrade vendor"
   Then the exit code is 1
   And Transaction is empty
   And stdout contains lines
       """
       Skipping 1 package due to vendor change restriction: "First Vendor" -> "Second Vendor".
       """
   And stderr contains "--allow-vendor-change to allow changing package vendors"


Scenario: Override default with --setopt=allow_vendor_change=true
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "upgrade --setopt=allow_vendor_change=true vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.2-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: Second Vendor"
