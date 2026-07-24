Feature: Tests for environment list and info commands


Background: Enable repo and mark an environment as installed
 Given I use repository "comps-group-list"
   # Create the installed environment manually so the test can be
   # used by dnf5daemon because it doesn't implement the environment
   # install command yet.
   And I create file "/usr/lib/sysimage/libdnf5/environments.toml" with
       """
       version = "1.0"
       [environments]

       test-environment = {groups = [], userinstalled = true}
       """

# https://github.com/rpm-software-management/dnf5/issues/1502
@xfail
Scenario: I can list environments (installed environment is not duplicated)
  When I execute dnf with args "environment list"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name              Installed
       test-environment     Test Environment        yes
       empty-environment    Empty Environment        no
       no-name-environment                           no
       """


Scenario: I can filter listed environments by their ids
  When I execute dnf with args "environment list "empty-*""
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name              Installed
       empty-environment    Empty Environment        no
       """


Scenario: I can filter listed environments by their names
  When I execute dnf with args "environment list "Empty *""
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name              Installed
       empty-environment    Empty Environment        no
       """


Scenario: I can list only installed environments
  When I execute dnf with args "environment list --installed"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name             Installed
       test-environment     Test Environment       yes
       """


Scenario: I can list only available environments
  When I execute dnf with args "environment list --available"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name              Installed
       test-environment     Test Environment         no
       empty-environment    Empty Environment        no
       no-name-environment                           no
       """


# https://github.com/rpm-software-management/dnf5/issues/1502
@xfail
Scenario: I can get info about all environments (installed environment is not duplicated)
  When I execute dnf with args "environment info"
  Then the exit code is 0
   And stderr is
      """
      <REPOSYNC>
      """
   And stdout is
      """
      Id                   : test-environment
      Name                 : Test Environment
      Description          : Test Environment description.
      Order                : 4
      Installed            : True
      Repositories         : @System
      Required groups      : test-group

      Id                   : empty-environment
      Name                 : Empty Environment
      Description          :
      Order                : 16
      Installed            : False
      Repositories         : comps-group-list

      Id                   : no-name-environment
      Name                 :
      Description          :
      Order                :
      Installed            : False
      Repositories         : comps-group-list
      Required groups      : test-group
      """


Scenario Outline: Info command accepts <option> option
  When I execute dnf with args "environment info <option>"
  Then the exit code is 0

Examples:
    | option                |
    | --installed           |
    | --available           |
