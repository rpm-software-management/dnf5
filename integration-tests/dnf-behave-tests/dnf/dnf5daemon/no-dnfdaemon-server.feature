@dnf5daemon
@destructive
@not.with_mode=dnf5
Feature: Test dnf5daemon-client initialization


# The test is flaky and dnf5daemon-client randomly returns either
# 'Failed to open bus (No such file or directory)' error message
# or 'Failed to open bus (Connection refused)'
@xfail
Scenario: Run dnf5daemon-client install when dbus is stopped
  Given I stop dbus
   When I execute dnf5daemon-client with args "repoquery"
   Then the exit code is 1
    And stderr is
    """
    Failed to open bus (No such file or directory)
    Is D-Bus daemon running?
    """


Scenario: Run dnf5daemon-client with no args when dbus is stopped
  Given I stop dbus
   When I execute dnf5daemon-client with no args
   Then the exit code is 2


# The test is flaky and dnf5daemon-client randomly returns either
# 'Failed to open bus (No such file or directory)' error message
# or 'Failed to open bus (Connection refused)'
@xfail
Scenario: Run dnf5daemon-client install when polkitd is stopped
  Given I stop polkitd
   When I execute dnf5daemon-client with args "repoquery rpm"
   Then the exit code is 1
    And stderr is
    """
    Failed to open bus (No such file or directory)
    Is D-Bus daemon running?
    """


Scenario: Run dnf5daemon-client with no args when polkitd is stopped
  Given I stop polkitd
   When I execute dnf5daemon-client with no args
   Then the exit code is 2
