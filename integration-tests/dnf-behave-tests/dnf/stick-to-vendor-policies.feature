Feature: DNF allow_vendor_change option with policies in .../vendors.d/*.conf TOML files


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


Scenario: Upgrade with vendor policy - allow change from First Vendor to Second Vendor
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
      """
      version = '1.0'

      [[outgoing_vendors]]
      vendor = 'First Vendor'

      [[incoming_vendors]]
      vendor = 'Second Vendor'
      """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.2-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: Second Vendor"


Scenario: Upgrade with vendor policy - allow change from First Vendor to Second Vendor (conf file in /usr)
  Given I create file "/usr/share/dnf5/vendors.d/test-policy.conf" with
      """
      version = '1.0'

      [[outgoing_vendors]]
      vendor = 'First Vendor'

      [[incoming_vendors]]
      vendor = 'Second Vendor'
      """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.2-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: Second Vendor"


Scenario: Upgrade with vendor policy - allow change from First Vendor to Second Vendor (conf in /etc overrides /usr)
  Given I create file "/usr/share/dnf5/vendors.d/test-policy.conf" with
      """
      version = '1.0'

      [[outgoing_vendors]]
      vendor = 'First Vendor'

      [[incoming_vendors]]
      vendor = 'Second Vendor'
      """
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
      """
      version = '1.0'

      [[outgoing_vendors]]
      vendor = 'First Vendor'

      [[incoming_vendors]]
      vendor = 'Third Vendor'
      """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.3-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: Third Vendor"


Scenario: Upgrade with vendor policy - equivalent vendors (mutual change allowed)
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
      """
      version = '1.0'

      [[equivalent_vendors]]
      vendor = 'First Vendor'

      [[equivalent_vendors]]
      vendor = 'Second Vendor'
      """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.2-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: Second Vendor"


Scenario: Upgrade with vendor policy - change for "First vendor" is not allowed
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
      """
      version = '1.0'

      [[outgoing_vendors]]
      vendor = 'Third Vendor'

      [[incoming_vendors]]
      vendor = 'Second Vendor'
      """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.1-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: First Vendor"


Scenario: Upgrade with vendor policy - glob pattern matching
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
      """
      version = '1.0'

      [[outgoing_vendors]]
      vendor = 'First Vendor'

      [[incoming_vendors]]
      vendor = '*Ve*or'
      comparator = 'GLOB'
      """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.3-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: Third Vendor"


Scenario: Upgrade with vendor policy - case insensitive matching and starts with
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
      """
      version = '1.0'

      [[outgoing_vendors]]
      vendor = 'first'
      comparator = 'ISTARTSWITH'

      [[incoming_vendors]]
      vendor = 'second vendor'
      comparator = 'IEXACT'
      """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.2-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: Second Vendor"


Scenario: Upgrade with vendor policy - using exclude to prevent specific vendor
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
      """
      version = '1.0'

      # Exclude "Third vendor"
      [[equivalent_vendors]]
      vendor = 'Third Vendor'
      exclude = true

      # All others vendors are equivalent (mutual change allowed)
      [[equivalent_vendors]]
      vendor = ''
      comparator = 'CONTAINS'
      """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade vendor"
   Then the exit code is 0
   And Transaction is following
       | Action  | Package             |
       | upgrade | vendor-1.2-1.x86_64 |
  Given I successfully execute rpm with args "-qi vendor"
   Then the exit code is 0
   And stdout contains "Vendor *: Second Vendor"
