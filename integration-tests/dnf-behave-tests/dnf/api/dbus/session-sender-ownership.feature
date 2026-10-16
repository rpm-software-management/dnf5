@dnf5daemon
@not.with_mode=dnf5
Feature: D-Bus session sender ownership

  Scenario: Reject session method calls from a different D-Bus sender
    When I execute python libdnf5 dbus api script with session
      """
      import os

      owner_repo = dbus.Interface(
          bus.get_object(DNFDAEMON_BUS_NAME, session),
          dbus_interface=IFACE_REPO)
      owner_repo.list({{}})

      foreign_bus = dbus.SystemBus(private=True)
      try:
          assert bus.get_unique_name() != foreign_bus.get_unique_name()

          foreign_repo = dbus.Interface(
              foreign_bus.get_object(DNFDAEMON_BUS_NAME, session),
              dbus_interface=IFACE_REPO)
          try:
              foreign_repo.list({{}})
          except dbus.exceptions.DBusException as error:
              assert error.get_dbus_name() == "org.rpm.dnf.v0.Error"
              assert "Session method can only be called by the D-Bus sender that opened the session." in str(error)
          else:
              raise AssertionError("foreign D-Bus sender was allowed to call a session method")

          foreign_rpm = dbus.Interface(
              foreign_bus.get_object(DNFDAEMON_BUS_NAME, session),
              dbus_interface=IFACE_RPM)
          read_fd, write_fd = os.pipe()
          try:
              foreign_rpm.list_fd({{}}, dbus.types.UnixFd(write_fd))
          except dbus.exceptions.DBusException as error:
              assert error.get_dbus_name() == "org.rpm.dnf.v0.Error"
              assert "Session method can only be called by the D-Bus sender that opened the session." in str(error)
          else:
              raise AssertionError("foreign D-Bus sender was allowed to call a file descriptor session method")
          finally:
              os.close(read_fd)
              os.close(write_fd)
      finally:
          foreign_bus.close()
          assert dbus.Boolean(True) == iface_session.close_session(session)
      """
    Then the exit code is 0
