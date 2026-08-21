@bz1639468
Feature: Reboot hint

Background:
    # We cannot use signed packages because we are moving the clock around,
    # rpm fails to read packages signed in the future.
    Given I use repository "unsigned" with configuration
        | key      | value |
        | gpgcheck | 0     |
      And I move the clock backward to "before boot-up"
      And I successfully execute dnf with args "install sarcina-1.0 kernel-1.0 glibc-1.0"
      And I move the clock forward to "the present"
      # Boot time is measured down to the second, so sleep for one second so
      # the time packages are installed is at least one second after container
      # boot
      And I sleep for "1" seconds

@bz1913962
Scenario: Update core packages
    Given I successfully execute dnf with args "upgrade kernel"
      And I successfully execute dnf with args "upgrade glibc"
     When I execute dnf with args "needs-restarting"
     Then the exit code is 1
      And stderr is
          """
          <REPOSYNC>
          """
      And stdout is
          """
          Core libraries or services have been updated since boot-up:
            * glibc
            * kernel

          Reboot is required to fully utilize these updates.
          More information: https://access.redhat.com/solutions/27943
          """

Scenario: Update core packages and check with --json
    Given I execute dnf with args "upgrade kernel"
      And I execute dnf with args "upgrade glibc"
     When I execute dnf with args "needs-restarting --json"
     Then the exit code is 1
      And stderr is
          """
          <REPOSYNC>
          """
      And stdout json matches
          """
          [{
            "type": "reboot",
            "reboot_required": true,
            "packages": [ "glibc", "kernel" ],
            "documentation": "https://access.redhat.com/solutions/27943"
          }]
          """

@bz1913962
Scenario: Install a package with an associated reboot_suggested advisory
    Given I execute dnf with args "upgrade --advisory=FEDORA-7777:003-03 \*"
     When I execute dnf with args "needs-restarting"
     Then the exit code is 1
      And stderr is
          """
          <REPOSYNC>
          """
      And stdout is
          """
          Core libraries or services have been updated since boot-up:
            * sarcina

          Reboot is required to fully utilize these updates.
          More information: https://access.redhat.com/solutions/27943
          """

Scenario: Update non-core packages only
    Given I execute dnf with args "upgrade lame basesystem wget"
     When I execute dnf with args "needs-restarting"
     Then the exit code is 0
      And stderr is
          """
          <REPOSYNC>
          """
      And stdout is
          """
          No core libraries or services have been updated since boot-up.
          Reboot should not be necessary.
          """

Scenario: -r short option (no-op for compatibility with DNF 4)
     When I execute dnf with args "needs-restarting -r"
     Then the exit code is 0

Scenario: --reboothint long option (no-op for compatibility with DNF 4)
     When I execute dnf with args "needs-restarting --reboothint"
     Then the exit code is 0
