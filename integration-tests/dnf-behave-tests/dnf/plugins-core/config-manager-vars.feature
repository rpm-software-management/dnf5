Feature: dnf "config-manager" command - test "setvar" and "unsetvar" subcommands


Scenario: set new variables
  Given I create directory "/etc/dnf/vars/"
   When I execute dnf with args "config-manager setvar mvar1=value1 mvar2=value2"
   Then the exit code is 0
    And file "/etc/dnf/vars/mvar1" contents is
      """
      value1
      """
      And file "/etc/dnf/vars/mvar2" contents is
      """
      value2
      """


Scenario: set variable multiple times times with the same value is OK
  Given I create directory "/etc/dnf/vars/"
   When I execute dnf with args "config-manager setvar mvar1=value1 mvar2=value2 mvar1=value1"
   Then the exit code is 0
    And file "/etc/dnf/vars/mvar1" contents is
      """
      value1
      """
      And file "/etc/dnf/vars/mvar2" contents is
      """
      value2
      """


Scenario: set variable more times with different value
  Given I create directory "/etc/dnf/vars/"
   When I execute dnf with args "config-manager setvar mvar1=value1 mvar2=value2 mvar1=other_value"
   Then the exit code is 1
    And stderr contains "Sets the "mvar1" variable again with a different value: "value1" != "other_value""


Scenario: set new variables, missing destination directory
   When I execute dnf with args "config-manager setvar mvar1=value1 mvar2=value2"
   Then the exit code is 1
    And stderr contains "Directory ".*/etc/dnf/vars" does not exist. Add "--create-missing-dir" to create missing directories"


Scenario: set new variables, "--create-missing-dir" creates missing destination directory
   When I execute dnf with args "config-manager setvar --create-missing-dir mvar1=value1 mvar2=value2"
   Then the exit code is 0
    And file "/etc/dnf/vars/mvar1" contents is
      """
      value1
      """
    And file "/etc/dnf/vars/mvar2" contents is
      """
      value2
     """


Scenario: set new variables, a file was found instead of a directory
  Given I create file "/etc/dnf/vars" with
    """
    """
   When I execute dnf with args "config-manager setvar mvar1=value1 mvar2=value2"
   Then the exit code is 1
    And stderr contains "The path ".*/etc/dnf/vars" exists, but it is not a directory or a symlink to a directory"


Scenario: set new variables, a symlink to non-existent object was found instead of a directory
  Given I create symlink "/etc/dnf/vars" to file "/non-exist"
   When I execute dnf with args "config-manager setvar mvar1=value1 mvar2=value2"
   Then the exit code is 1
    And stderr contains "The path ".*/etc/dnf/vars" exists, but it is a symlink to a non-existent object"


Scenario: change the value of an existing variable
  Given I create file "/etc/dnf/vars/mvar1" with
      """
      orig_value1
      """
    And I execute dnf with args "config-manager setvar mvar1=value1"
   Then the exit code is 0
    And file "/etc/dnf/vars/mvar1" contents is
      """
      value1
      """


Scenario: unset/remove an existing variable
  Given I create file "/etc/dnf/vars/mvar1" with
      """
      orig_value1
      """
    And I create file "/etc/dnf/vars/mvar2" with
      """
      orig_value2
      """
    And I execute dnf with args "config-manager unsetvar mvar1"
   Then the exit code is 0
    And file "/etc/dnf/vars/mvar1" does not exist
    And file "/etc/dnf/vars/mvar2" contents is
      """
      orig_value2
      """


Scenario: unset/remove an existing variable, removing non-existent variable is OK, but a warning is written
  Given I create file "/etc/dnf/vars/mvar1" with
      """
      orig_value1
      """
    And I create file "/etc/dnf/vars/mvar2" with
      """
      orig_value2
      """
    And I execute dnf with args "config-manager unsetvar mvar1 nonexistvar"
   Then the exit code is 0
    And file "/etc/dnf/vars/mvar1" does not exist
    And file "/etc/dnf/vars/mvar2" contents is
      """
      orig_value2
      """
    And stderr is
      """
      config-manager: Request to remove variable but it is not present in the vars directory: nonexistvar
      """


Scenario: removing non-existent variable (directory with variables not found) is OK, but a warning is written
   When I execute dnf with args "config-manager unsetvar nonexistvar"
   Then the exit code is 0
    And stderr is
      """
      config-manager: Request to remove variable but vars directory was not found: {context.dnf.installroot}/etc/dnf/vars
      """
