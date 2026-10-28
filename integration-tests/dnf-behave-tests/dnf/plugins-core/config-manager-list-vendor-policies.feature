Feature: dnf "config-manager" command - test "list-vendor-policies" subcommand


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


Scenario: test "list-vendor-policies"
   When I execute dnf with args "config-manager list-vendor-policies"
   Then the exit code is 0
    And stdout is
        """
        * {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-cmdline.conf
        + {context.dnf.installroot}/etc/dnf/vendors.d/allow-my_vendor.conf
        + {context.dnf.installroot}/etc/dnf/vendors.d/allow-test.conf
        - {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-test.conf
        + {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """


Scenario: test "list-vendor-policies" with --name='*test'
   When I execute dnf with args "config-manager list-vendor-policies --name='*test'"
   Then the exit code is 0
    And stdout is
        """
        + {context.dnf.installroot}/etc/dnf/vendors.d/allow-test.conf
        - {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-test.conf
        + {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """


Scenario: test "list-vendor-policies" with --info
   When I execute dnf with args "config-manager list-vendor-policies --info"
   Then the exit code is 0
    And stdout is
        """
        Policy  : allow-cmdline
        Source  : {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-cmdline.conf
        Type    : Distribution (read-only)
        Compact : @in:[cmdline_repo="1"]

        Policy  : allow-my_vendor
        Source  : {context.dnf.installroot}/etc/dnf/vendors.d/allow-my_vendor.conf
        Type    : System (manageable)
        Compact : in:"My Vendor"

        Policy  : allow-test
        Source  : {context.dnf.installroot}/etc/dnf/vendors.d/allow-test.conf
        Type    : System (manageable)
        Masks   : {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-test.conf
        Compact :

        Policy    : allow-test
        Source    : {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-test.conf
        Type      : Distribution (read-only)
        Masked by : {context.dnf.installroot}/etc/dnf/vendors.d/allow-test.conf
        Compact   : in:"Test Vendor"

        Policy  : test
        Source  : {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        Type    : System (manageable)
        Compact : in:=*"Enter*",in:=*"Original Co*",out:^"New Maintainer L",eq:"Fedora",eq:"CentOS"@in:[name="iftop",arch="x86_64"],out:[name="iftop"]
        """


Scenario: test "list-vendor-policies" with --info and name='*test'
   When I execute dnf with args "config-manager list-vendor-policies --info --name='*test'"
   Then the exit code is 0
    And stdout is
        """
        Policy  : allow-test
        Source  : {context.dnf.installroot}/etc/dnf/vendors.d/allow-test.conf
        Type    : System (manageable)
        Masks   : {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-test.conf
        Compact :

        Policy    : allow-test
        Source    : {context.dnf.installroot}/usr/share/dnf5/vendors.d/allow-test.conf
        Type      : Distribution (read-only)
        Masked by : {context.dnf.installroot}/etc/dnf/vendors.d/allow-test.conf
        Compact   : in:"Test Vendor"

        Policy  : test
        Source  : {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        Type    : System (manageable)
        Compact : in:=*"Enter*",in:=*"Original Co*",out:^"New Maintainer L",eq:"Fedora",eq:"CentOS"@in:[name="iftop",arch="x86_64"],out:[name="iftop"]
        """
