@dnf5daemon
@not.with_mode=dnf5
Feature: D-Bus api: rpm install

Background: Prepare repositories and a daemon session
  Given I use repository "dbus-api-base"
    And I use repository "dbus-api-second"
    And I open a dnf5daemon session

Scenario: Install a package
   When I install packages "one" via D-Bus
    And I resolve the D-Bus goal
   Then the D-Bus transaction is
        | action  | reason | full_nevra       | repo_id       | from_repo_id | package_reason |
        | Install | User   | one-0:2-1.noarch | dbus-api-base |              | None           |
   When I run the D-Bus transaction
    And I execute rpm with args "-q one"
   Then the exit code is 0
    And stdout is
        """
        one-2-1.noarch
        """

Scenario: Install of a nonexistent package without skip_unavailable
   When I install packages "no_one" via D-Bus with options
        | key             | value |
        | skip_unavailable | False |
    And I resolve the D-Bus goal
   Then the D-Bus goal resolve result is 2
    And the D-Bus transaction is empty
    And the D-Bus transaction problems are
        """
        No match for argument: no_one
        """

Scenario: Install of a nonexistent package with skip_unavailable
   When I install packages "no_one" via D-Bus with options
        | key             | value |
        | skip_unavailable | True |
    And I resolve the D-Bus goal
   Then the D-Bus goal resolve result is 1
    And the D-Bus transaction is empty
    And the D-Bus transaction problems are
        """
        No match for argument: no_one
        """

Scenario: Install restricted to a repository that does not contain the package
   When I install packages "one" via D-Bus with options
        | key      | value           |
        | repo_ids | dbus-api-second |
    And I resolve the D-Bus goal
   Then the D-Bus goal resolve result is 2
    And the D-Bus transaction is empty
    And the D-Bus transaction problems are
        """
        No match for argument 'one' in repositories 'dbus-api-second'
        """

Scenario: Install a package from an rpm file on disk
   When I install packages "{context.dnf.fixturesdir}/repos/dbus-api-base/noarch/one-1-1.noarch.rpm" via D-Bus
    And I resolve the D-Bus goal
   Then the D-Bus transaction is
        | action  | reason | full_nevra       | repo_id      | from_repo_id | package_reason |
        | Install | User   | one-0:1-1.noarch | @commandline |              | None           |
   When I run the D-Bus transaction
    And I execute rpm with args "-q one"
   Then the exit code is 0
    And stdout is
        """
        one-1-1.noarch
        """

Scenario: Resolve a file install multiple times in the same session
   When I execute python libdnf5 dbus api script with session
        """
        # Regression test for "Id is out of bitmap range"
        # (https://github.com/rpm-software-management/dnf5/issues/2756):
        # GoalPrivate::operator= did not reset the transaction bitmaps, so
        # repeated goal reset / file install / resolve cycles in a single
        # session crashed once package ids outgrew the stale, byte-aligned
        # bitmap. Each cycle adds one new sack id; 16 cycles guarantee
        # crossing the bitmap boundary regardless of id alignment.
        rpm_path = "{context.dnf.fixturesdir}/repos/dbus-api-base/noarch/one-1-1.noarch.rpm"
        iface_rpm = dbus.Interface(
            bus.get_object(DNFDAEMON_BUS_NAME, session),
            dbus_interface=IFACE_RPM)
        iface_goal = dbus.Interface(
            bus.get_object(DNFDAEMON_BUS_NAME, session),
            dbus_interface=IFACE_GOAL)
        for i in range(16):
            iface_goal.reset({{}})
            iface_rpm.install([rpm_path], {{}})
            resolved, result = iface_goal.resolve({{}})
            assert result == 0, "Resolve failed on iteration %d" % i
        """
   Then the exit code is 0
