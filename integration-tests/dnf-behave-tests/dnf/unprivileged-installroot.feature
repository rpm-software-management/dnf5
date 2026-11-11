@destructive
@no_installroot
Feature: Installroot test as unprivileged user


# Additional details https://github.com/rpm-software-management/dnf5/issues/2028
Scenario: Fail when installing into installroot as unprivileged user
  # The default-key is not pre-imported into the custom /home/testuser/f32 installroot
  Given I use repository "miscellaneous" with configuration
        | key           | value                                                                        |
        | gpgkey        | file://{context.dnf.fixturesdir}/gpgkeys/keys/default-key/default-key-public |
   When I execute dnf with args "--releasever=32 --installroot=/home/testuser/f32 --use-host-config --setopt=cachedir=/home/testuser/f32 install dummy" as an unprivileged user
   Then the exit code is 1
    And stderr contains "Transaction failed: Rpm transaction failed."
    And stderr contains "- Unable to change root directory: Operation not permitted"


@bz1843280
Scenario: Fail when missing permissions for installroot directory
  Given I use repository "miscellaneous"
   When I execute dnf with args "--releasever=32 --installroot=/var/lib/f32 install dummy" as an unprivileged user
   Then the exit code is 1
    And stderr is
        """
        filesystem error: cannot create directories: Permission denied [/var/lib/f32/home/testuser/.local/state]
        """
