Feature: DNF allow_vendor_change option with policies version "1.1" in .../vendors.d/*.conf TOML files


Background:
  Given I use repository "dnf-ci-vendor-1"
    And I create and substitute file "/etc/dnf/dnf.conf" with
    """
    [main]
    allow_vendor_change=False
    """
    And I successfully execute dnf with args "install vendor vendorpkg"
    Then the exit code is 0
  Given I successfully execute rpm with args "--queryformat='%{{name}}-%{{version}}-%{{release}} : %{{vendor}}\n' -q -a"
   Then the exit code is 0
    And stdout contains lines
    """
    vendor-1.0-1 : First Vendor
    vendorpkg-1.0-1 : First Vendor
    vendordep-1.0-1 : First Vendor
    """


Scenario: Upgrade with vendor policy - equivalent vendors and incoming vendors together
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
    """
    version = '1.1'

    [[equivalent_vendors]]
    vendor = 'First Vendor'

    [[equivalent_vendors]]
    vendor = 'Second Vendor'

    [[incoming_vendors]]
    vendor = 'Third Vendor'
    """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade '*'"
   Then the exit code is 0
  Given I successfully execute rpm with args "--queryformat='%{{name}}-%{{version}}-%{{release}} : %{{vendor}}\n' -q -a"
   Then the exit code is 0
    And stdout contains lines
    """
    vendor-1.3-1 : Third Vendor
    vendorpkg-1.12-1 : Third Vendor
    vendordep-1.8-1 : Third Vendor
    vendordep2-1.2-1 : Third Vendor
    """


Scenario: Upgrade with vendor policy - any vendor change to "Second vendor" is allowed
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
    """
    version = '1.1'

    [[incoming_vendors]]
    vendor = 'Second Vendor'
    """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade '*'"
   Then the exit code is 0
  Given I successfully execute rpm with args "--queryformat='%{{name}}-%{{version}}-%{{release}} : %{{vendor}}\n' -q -a"
   Then the exit code is 0
    And stdout contains lines
    """
    vendor-1.2-1 : Second Vendor
    vendorpkg-1.10-1 : Second Vendor
    vendordep-1.6-1 : First Vendor
    vendordep2-1.1-1 : Second Vendor
    vendordep3-1.2-1 : Third Vendor
    """


Scenario: Upgrade with vendor policy - Allow packages from the command-line repo from any vendor
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
    """
    version = '1.1'

    [[incoming_packages]]
    filters = [
      { filter = 'cmdline_repo', value = 'true' }
    ]
    """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade {context.dnf.fixturesdir}/repos/dnf-ci-vendor-2-updates/x86_64/vendor-1.2-1.x86_64.rpm {context.dnf.fixturesdir}/repos/dnf-ci-vendor-3-updates/x86_64/vendorpkg-1.12-1.x86_64.rpm"
   Then the exit code is 0
  Given I successfully execute rpm with args "--queryformat='%{{name}}-%{{version}}-%{{release}} : %{{vendor}}\n' -q -a"
   Then the exit code is 0
    And stdout contains lines
    """
    vendor-1.2-1 : Second Vendor
    vendorpkg-1.12-1 : Third Vendor
    vendordep-1.0-1 : First Vendor
    vendordep2-1.2-1 : Third Vendor
    """


Scenario: Upgrade with vendor policy - Allow packages from the command-line repo from any vendor with exclusion packages whose name ends with "pkg"
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
    """
    version = '1.1'

    [[incoming_packages]]
    filters = [
      { filter = 'name', value = 'pkg', comparator = 'ENDSWITH' }
    ]
    exclude = true

    [[incoming_packages]]
    filters = [
      { filter = 'cmdline_repo', value = '1' }
    ]
    """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade {context.dnf.fixturesdir}/repos/dnf-ci-vendor-2-updates/x86_64/vendor-1.2-1.x86_64.rpm {context.dnf.fixturesdir}/repos/dnf-ci-vendor-3-updates/x86_64/vendorpkg-1.12-1.x86_64.rpm"
   Then the exit code is 0
  Given I successfully execute rpm with args "--queryformat='%{{name}}-%{{version}}-%{{release}} : %{{vendor}}\n' -q -a"
   Then the exit code is 0
    And stdout contains lines
    """
    vendor-1.2-1 : Second Vendor
    vendorpkg-1.0-1 : First Vendor
    vendordep-1.0-1 : First Vendor
    """


Scenario: Upgrade with vendor policy - all incoming packages whose source name starting "vend" and version is >= "1.8" to change vendor to "Third Vendor"
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
    """
    version = '1.1'

    [[incoming_packages]]
    filters = [
      { filter = 'source_name', value = 'vend', comparator = 'STARTSWITH' },
      { filter = 'version', value = '1.8', comparator = 'GTE' }
    ]

    [[incoming_vendors]]
    vendor = 'Third Vendor'
    """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade '*'"
   Then the exit code is 0
  Given I successfully execute rpm with args "--queryformat='%{{name}}-%{{version}}-%{{release}} : %{{vendor}}\n' -q -a"
   Then the exit code is 0
    And stdout contains lines
    """
    vendor-1.1-1 : First Vendor
    vendorpkg-1.12-1 : Third Vendor
    vendordep-1.8-1 : Third Vendor
    vendordep2-1.2-1 : Third Vendor
    """
