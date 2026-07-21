Feature: DNF allow_vendor_change option in dnf.conf
Background:
  Given I use repository "dnf-ci-vendor-1"
   And I create and substitute file "/etc/dnf/dnf.conf" with
   """
   [main]
   allow_vendor_change=False
   """
   And I successfully execute dnf with args "install vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | install | vendor-1.0-1.x86_64 |

  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: First Vendor"

@bz1788371
Scenario: Upgrade sticks to vendor
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

@bz1788371
Scenario: No upgrade if same vendor not found
  Given I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is empty
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: First Vendor"


@bz1788371
Scenario: Downgrade is unable to resolve transaction
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
   And stderr is
       """
       <REPOSYNC>
       Failed to resolve the transaction:
       Problem: problem with installed package
         - cannot install both vendor-1.0-1.x86_64 from dnf-ci-vendor-2 and vendor-1.1-1.x86_64 from @System
         - cannot install both vendor-1.0-1.x86_64 from dnf-ci-vendor-2 and vendor-1.1-1.x86_64 from dnf-ci-vendor-1-updates
         - conflicting requests
       You can try to add to command line:
         --skip-broken to skip uninstallable packages
         --allow-vendor-change to allow changing package vendors
       """
