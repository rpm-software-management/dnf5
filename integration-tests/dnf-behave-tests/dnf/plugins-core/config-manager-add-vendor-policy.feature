Feature: dnf "config-manager" command - test "add-vendor-policy" subcommand


Scenario: test "add-vendor-policy" from --policy, destination directory exists
  Given I create directory "/etc/dnf/vendors.d/"
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\""
   Then the exit code is 0
    And stderr is
        """
        Created vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy" from --policy, with --create-missing-dir, destination directory exists
  Given I create directory "/etc/dnf/vendors.d/"
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --create-missing-dir"
   Then the exit code is 0
    And stderr is
        """
        Created vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy" from --policy, destination directory does not exist
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\""
   Then the exit code is 1
    And stderr contains "No such file or directory"


Scenario: test "add-vendor-policy" from --policy, with --create-missing-dir, destination directory does not exist
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --create-missing-dir"
   Then the exit code is 0
    And stderr is
        """
        Created vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy" from local file, with --create-missing-dir, destination directory does not exist
  Given I create file "/{context.dnf.tempdir}/tmp/vpolicy.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --from-tomlfile={context.dnf.tempdir}/tmp/vpolicy.conf --create-missing-dir"
   Then the exit code is 0
    And stderr is
        """
        Saved vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy" from local file via file:///, with --create-missing-dir, destination directory does not exist
  Given I create file "/{context.dnf.tempdir}/tmp/vpolicy.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --from-tomlfile=file://{context.dnf.tempdir}/tmp/vpolicy.conf --create-missing-dir"
   Then the exit code is 0
    And stderr contains "Saved vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf"
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy" from remote file via http://, with --create-missing-dir, destination directory does not exist
  Given I create directory "/remotedir"
    And I create file "/remotedir/vpolicy.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """
    And I set up a http server for directory "/remotedir"
   When I execute dnf with args "config-manager add-vendor-policy --name=test --from-tomlfile=http://localhost:{context.dnf.ports[/remotedir]}/vpolicy.conf --create-missing-dir"
   Then the exit code is 0
    And stderr contains "Saved vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf"
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy", fail because distribution policy exists
  Given I create file "/usr/share/dnf5/vendors.d/test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "distrib_new_vendor"

        [[outgoing_vendors]]
        vendor = "distrib_old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --create-missing-dir"
   Then the exit code is 1
    And stderr contains "Distribution policy "test" already exists. Use \"--allow-mask\" to allow masking it."


Scenario: test "add-vendor-policy", use --allow-mask, distribution policy exists
  Given I create file "/usr/share/dnf5/vendors.d/test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "distrib_new_vendor"

        [[outgoing_vendors]]
        vendor = "distrib_old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --allow-mask --create-missing-dir"
   Then the exit code is 0
    And stderr is
        """
        Created vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        Masked distribution policy file: {context.dnf.installroot}/usr/share/dnf5/vendors.d/test.conf
        """
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy", use --mask, distribution policy exists
  Given I create file "/usr/share/dnf5/vendors.d/test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "distrib_new_vendor"

        [[outgoing_vendors]]
        vendor = "distrib_old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --mask --create-missing-dir"
   Then the exit code is 0
    And stderr is
        """
        Created vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        Masked distribution policy file: {context.dnf.installroot}/usr/share/dnf5/vendors.d/test.conf
        """
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy", use --mask, fail because distribution policy does not exist
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --mask --create-missing-dir"
   Then the exit code is 1
    And stderr contains "Cannot add policy: option "--mask" requires masking a distribution policy, but no distribution policy named "test" exists."


Scenario: test "add-vendor-policy", fail because policy exists
  Given I create file "/etc/dnf/vendors.d/test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "distrib_new_vendor"

        [[outgoing_vendors]]
        vendor = "distrib_old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --create-missing-dir"
   Then the exit code is 1
    And stderr contains "{context.dnf.installroot}/etc/dnf/vendors.d/test.conf\" already exists. Use \"--allow-replace\" to allow replacing it."


Scenario: test "add-vendor-policy", use --allow-replace, policy exists
  Given I create file "/etc/dnf/vendors.d/test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "distrib_new_vendor"

        [[outgoing_vendors]]
        vendor = "distrib_old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --allow-replace --create-missing-dir"
   Then the exit code is 0
    And stderr is
        """
        Replaced vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy", use --replace, policy exists
  Given I create file "/etc/dnf/vendors.d/test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "distrib_new_vendor"

        [[outgoing_vendors]]
        vendor = "distrib_old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --replace --create-missing-dir"
   Then the exit code is 0
    And stderr is
        """
        Replaced vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        """
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy", use --replace, fail because policy does not exist
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --replace --create-missing-dir"
   Then the exit code is 1
    And stderr contains "Cannot add policy: option "--replace" requires replacing an existing system policy, but no system policy file named "test" exists."


Scenario: test "add-vendor-policy", use --allow-replace, distribution and system policies exist
  Given I create file "/usr/share/dnf5/vendors.d/test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "distrib_new_vendor"

        [[outgoing_vendors]]
        vendor = "distrib_old_vendor"
        """
    And I create file "/etc/dnf/vendors.d/test.conf" with
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "distrib_new_vendor"

        [[outgoing_vendors]]
        vendor = "distrib_old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",out:\"old_vendor\" --allow-replace --create-missing-dir"
   Then the exit code is 0
    And stderr is
        """
        Replaced vendor policy file: {context.dnf.installroot}/etc/dnf/vendors.d/test.conf
        Masked distribution policy file: {context.dnf.installroot}/usr/share/dnf5/vendors.d/test.conf
        """
    And file "/etc/dnf/vendors.d/test.conf" contents is
        """
        version = '1.2'

        [[incoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """


Scenario: test "add-vendor-policy" from --policy, with invalid policy string
   When I execute dnf with args "config-manager add-vendor-policy --name=test --policy=in:\"new_vendor\",,out:\"old_vendor\" --create-missing-dir"
   Then the exit code is 1
    And stderr is
        """
        Failed to add vendor policy
         Compact vendor change policy parse error in "text:COMMAND LINE" at position 16
          Expected direction prefix 'in', 'out', or 'eq'
        """


Scenario: test "add-vendor-policy" from local TOML file, with invalid content
  Given I create file "/{context.dnf.tempdir}/tmp/vpolicy.conf" with
        """
        version = '1.2'

        [[xincoming_vendors]]
        vendor = "new_vendor"

        [[outgoing_vendors]]
        vendor = "old_vendor"
        """
   When I execute dnf with args "config-manager add-vendor-policy --name=test --from-tomlfile={context.dnf.tempdir}/tmp/vpolicy.conf --create-missing-dir"
   Then the exit code is 1
    And stderr is
        """
        Failed to add vendor policy
         Unknown key 'xincoming_vendors' in "file://{context.dnf.tempdir}/tmp/vpolicy.conf" on line 3
        """
