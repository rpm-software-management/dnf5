@destructive
@no_installroot
Feature: Installroot test as unprivileged user


# Reported as https://github.com/rpm-software-management/dnf5/issues/2028
@xfail
Scenario: Fail when installing into installroot as unprivileged user
  Given I use repository "miscellaneous"
   When I execute dnf with args "--releasever=32 --installroot=/home/testuser/f32 install dummy" as an unprivileged user
   Then the exit code is 1
    And stderr is
        """
        Error: This command has to be run with superuser privileges (under the root user on most systems).
        """


@bz1843280
Scenario: Fail when missing permissions for installroot directory
  Given I use repository "miscellaneous"
   When I execute dnf with args "--releasever=32 --installroot=/var/lib/f32 install dummy" as an unprivileged user
   Then the exit code is 1
    And stderr is
        """
        filesystem error: cannot create directories: Permission denied [/var/lib/f32/home/testuser/.local/state]
        """
