# It is not clear if this should be supported,
# reported as: https://github.com/rpm-software-management/dnf5/issues/1815
@xfail
Feature: Add package to needs-restarting using config files

Background:
Given I use repository "needs-restarting"
  And I move the clock backward to "before boot-up"
  And I execute dnf with args "install wget abcde"
  And I move the clock forward to "2 hours"
  And I use repository "needs-restarting-updates"
  And I create directory "/etc/dnf/plugins/needs-restarting.d"
  And I create and substitute file "/etc/dnf/plugins/needs-restarting.d/wget.conf" with
  """
  wget
  """


@bz1810123
Scenario: plugin does not fail if config directory not found
Given I delete directory "/etc/dnf/plugins/needs-restarting.d/"
 When I execute dnf with args "needs-restarting -r"
 Then the exit code is 0
  And stdout is
  """
  No core libraries or services have been updated since boot-up.
  Reboot should not be necessary.
  """

@bz1810123
Scenario: restarting needed after updating package in conf files
Given I execute dnf with args "upgrade wget"
 When I execute dnf with args "needs-restarting -r"
 Then the exit code is 1
  And stdout is
  """
  Core libraries or services have been updated since boot-up:
    * wget

  Reboot is required to fully utilize these updates.
  More information: https://access.redhat.com/solutions/27943
  """

@bz1810123
Scenario: conf files can be more than one
Given I create and substitute file "/etc/dnf/plugins/needs-restarting.d/abcde.conf" with
  """
  abcde
  """
  And I successfully execute dnf with args "upgrade wget abcde"
 When I execute dnf with args "needs-restarting -r"
 Then the exit code is 1
  And stdout is
  """
  Core libraries or services have been updated since boot-up:
    * abcde
    * wget

  Reboot is required to fully utilize these updates.
  More information: https://access.redhat.com/solutions/27943
  """

@bz1810123
Scenario: conf files can contain more than one option
Given I create and substitute file "/etc/dnf/plugins/needs-restarting.d/wget.conf" with
  """
  wget
  abcde
  """
  And I successfully execute dnf with args "upgrade wget abcde"
 When I execute dnf with args "needs-restarting -r"
 Then the exit code is 1
  And stdout is
  """
  Core libraries or services have been updated since boot-up:
    * abcde
    * wget

  Reboot is required to fully utilize these updates.
  More information: https://access.redhat.com/solutions/27943
  """


@bz1810123
Scenario: conf files without .conf extension are ignored
Given I create and substitute file "/etc/dnf/plugins/needs-restarting.d/abcde" with
  """
  abcde
  """
  And I successfully execute dnf with args "upgrade abcde"
 When I execute dnf with args "needs-restarting -r"
 Then the exit code is 0
  And stdout is
  """
  No core libraries or services have been updated since boot-up.
  Reboot should not be necessary.
  """

@bz1810123
Scenario: user gets a warning when program is not installed
Given I create and substitute file "/etc/dnf/plugins/needs-restarting.d/abcde.conf" with
  """
  abcde
  """
  And I successfully execute dnf with args "remove abcde"
  And I successfully execute dnf with args "upgrade wget"
 When I execute dnf with args "needs-restarting -r"
 Then the exit code is 1
  And stdout is
  """
  Core libraries or services have been updated since boot-up:
    * wget

  Reboot is required to fully utilize these updates.
  More information: https://access.redhat.com/solutions/27943
  """
  And stderr is
  """
  No installed package found for package name "abcde" specified in needs-restarting file "abcde.conf".
  """


@bz1810123
Scenario: package self adds to needs-restarting
Given I successfully execute dnf with args "install dwm"
 When I execute dnf with args "needs-restarting -r"
 Then the exit code is 1
  And stdout is
  """
  Core libraries or services have been updated since boot-up:
    * dwm

  Reboot is required to fully utilize these updates.
  More information: https://access.redhat.com/solutions/27943
  """
