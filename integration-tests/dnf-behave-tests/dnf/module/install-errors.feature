# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Installing module profiles - error handling

Background:
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"


Scenario: A proper error message is displayed when I try to install a non-existent module
   When I execute dnf with args "module install NonExistentModule"
   Then the exit code is 1
    And stderr contains lines
    """
    Error: Problems in request:
    missing groups or modules: NonExistentModule
    """

Scenario: A proper error message is displayed when I try to install a non-existent module using group syntax
   When I execute dnf with args "install @NonExistentModule"
   Then the exit code is 1
    And stderr contains lines
    """
    Module or Group 'NonExistentModule' is not available.
    Error: Nothing to do.
    """


Scenario: I cannot install an RPM with same name as an RPM that belongs to enabled MODULE:STREAM
   When I execute dnf with args "module disable ninja"
   Then the exit code is 0
   When I execute dnf with args "install ninja-build-0:1.8.2-5.fc29.x86_64"
   Then the exit code is 0
   When I execute dnf with args "remove ninja-build"
   Then the exit code is 0
   When I execute dnf with args "module enable ninja:master"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                       |
        | module-stream-enable      | ninja:master                                  |
   When I execute dnf with args "install ninja-build-0:1.8.2-5.fc29.x86_64"
   Then the exit code is 1
    And stderr contains lines
    """
    Error: Unable to find a match: ninja-build-0:1.8.2-5.fc29.x86_64
    """
    And stdout contains lines
    """
    All matches were filtered out by modular filtering for argument: ninja-build-0:1.8.2-5.fc29.x86_64
    """

Scenario: A proper error message is displayed when I try to install a non-existent stream
 When I execute dnf with args "module install meson:NoSuchStream"
 Then the exit code is 1
  And stderr contains lines
  """
  Error: Problems in request:
  missing groups or modules: meson:NoSuchStream
  """


@bz1724564
@bz1790967
Scenario: Install module without any profiles
   When I execute dnf with args "module install DnfCiModuleNoProfiles:master"
   Then the exit code is 1
    And Transaction is empty
    And modules state is following
        | Module                  | State     | Stream    | Profiles  |
        | DnfCiModuleNoProfiles   |           |           |           |
    And stderr is
        """
        No profiles for module DnfCiModuleNoProfiles:master
        Error: Problems in request:
        broken groups or modules: DnfCiModuleNoProfiles:master
        """


@bz1645167
Scenario: A proper error message is displayed when I try to install a non-existent profile
 When I execute dnf with args "module install meson:master/NoSuchProfile"
 Then the exit code is 1
  And stderr contains lines
  """
  Error: Problems in request:
  missing groups or modules: meson:master/NoSuchProfile
  """


Scenario: Install fails even when only one of the module specs cannot be installed
   When I execute dnf with args "module install nodejs:8/minimal meson:master/NoSuchProfile"
   Then the exit code is 1
    And Transaction is empty
    And stderr contains lines
        """
        Error: Problems in request:
        missing groups or modules: meson:master/NoSuchProfile
        """


# package FileConflict-1.0-1.x86_64 has file conflicts with
# FileConflict-0:2.0.streamB-1.x86_64 from module test-module
@bz1719679
Scenario: Profile is not installed after its artifact failed to get installed
  Given I use repository "dnf-ci-fileconflicts"
   When I execute dnf with args "install FileConflict-1.0-1.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                           |
        | install                   | FileConflict-0:1.0-1.x86_64  |
   When I execute dnf with args "module install test-module:B/default"
   Then the exit code is 1
    And stderr is
    """
    Error: Transaction test error:
      file /usr/lib/FileConflict/a_dir from install of FileConflict-0:2.0.streamB-1.x86_64 conflicts with file from package FileConflict-0:1.0-1.x86_64
    """
    And modules state is following
        | Module        | State     | Stream    | Profiles  |
        | test-module   |           |           |           |
