Feature: dnf "config-manager" command - test "remove-vendor-policy" subcommand


Background:
  Given I create file "/usr/share/dnf5/vendors.d/allow-cmdline.conf" with
        """
        version = '1.2'

        [[incoming_packages]]
        filters = [
          { filter = 'cmdline_repo', value = 'true' }
        ]
        """
    And I create file "/etc/dnf/vendors.d/allow-my_vendor.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "My Vendor"
        """
    And I create file "/etc/dnf/vendors.d/allow-test.conf" with
        """
        version = '1.2'
        """
    And I create file "/usr/share/dnf5/vendors.d/allow-test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "Test Vendor"
        """
    And I create file "/etc/dnf/vendors.d/test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "Enter*"
        comparator = 'GLOB'

        [[incoming_vendors]]
        vendor = "Original Co*"
        comparator = 'GLOB'

        [[outgoing_vendors]]
        vendor = "New Maintainer L"
        comparator = 'STARTSWITH'

        [[equivalent_vendors]]
        vendor = "Fedora"

        [[equivalent_vendors]]
        vendor = "CentOS"

        [[incoming_packages]]
        filters = [
          { filter = 'name', value = "iftop" },
          { filter = 'arch', value = "x86_64" }
        ]

        [[outgoing_packages]]
        filters = [
          { filter = 'name', value = "iftop" }
        ]
        """


Scenario: test "remove-vendor-policy" with --name=test
   When I execute dnf with args "config-manager remove-vendor-policy --name=test"
   Then the exit code is 0
    And stderr is
        """
        Removed vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """


Scenario: test "remove-vendor-policy" with --name='*test'
   When I execute dnf with args "config-manager remove-vendor-policy --name='*test'"
   Then the exit code is 0
    And stderr is
        """
        Removed vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/allow-test.conf
        Unmasked distribution policy file: {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-test.conf
        Removed vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """


Scenario: test "remove-vendor-policy", cannot remove distribution policy
   When I execute dnf with args "config-manager remove-vendor-policy --name=allow-cmdline"
   Then the exit code is 0
    And stderr is
        """
        Cannot remove distribution policy file "{context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-cmdline.conf". To mask it, use: dnf5 config-manager add-vendor-policy --name='allow-cmdline' --mask --policy=
        """


Scenario: test "remove-vendor-policy", with --name='allow*'
   When I execute dnf with args "config-manager remove-vendor-policy --name='allow*'"
   Then the exit code is 0
    And stderr is
        """
        Cannot remove distribution policy file "{context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-cmdline.conf". To mask it, use: dnf5 config-manager add-vendor-policy --name='allow-cmdline' --mask --policy=
        Removed vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/allow-my_vendor.conf
        Removed vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/allow-test.conf
        Unmasked distribution policy file: {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-test.conf
        """


Scenario: test "remove-vendor-policy", with name=nonexist
   When I execute dnf with args "config-manager remove-vendor-policy --name=nonexist"
   Then the exit code is 1
    And stderr is
        """
        No matching vendor policy files found for pattern: nonexist
        """


Scenario: test "remove-vendor-policy", with name='nonexist*'
   When I execute dnf with args "config-manager remove-vendor-policy --name='nonexist*'"
   Then the exit code is 1
    And stderr is
        """
        No matching vendor policy files found for pattern: nonexist*
        """
