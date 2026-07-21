# aliases configuration directories are always taken from the host
@destructive
Feature: Test for dnf5 aliases functionality

Background:
  Given I use repository "simple-base"


Scenario Outline: DNF recognizes command alias created in <aliases_path> directory
Given I create directory "/<aliases_path>"
  And I create file "/<aliases_path>/TEST_ALIASES.conf" with
"""
version = '1.0'

['inthrone']
type = 'command'
attached_command = 'install'
descr = 'Install command test alias'
"""
 When I execute dnf with args "inthrone labirinto"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |

Examples:
    | aliases_path                  |
    | /usr/share/dnf5/aliases.d     |
    | /etc/dnf/dnf5-aliases.d       |
    | /root/.config/dnf5/aliases.d  |


Scenario: I can add option to the aliased command
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.0'

['inthrone']
type = 'command'
attached_command = 'install'
descr = 'Install command test alias'
attached_named_args = [
    { id_path = 'assumeno' }
]
"""
 When I execute dnf with args "inthrone labirinto"
 Then the exit code is 1
  And stderr contains "Operation aborted by the user."


Scenario: I can add option with value to the aliased command
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.0'

['inthrone']
type = 'command'
attached_command = 'install'
descr = 'Install command test alias'
attached_named_args = [
    { id_path = 'repo', value = 'does_not_exist' }
]
"""
 When I execute dnf with args "inthrone labirinto"
 Then the exit code is 2
  And stderr contains "No matching repositories for does_not_exist."


Scenario: DNF recognizes alias for a named argument
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.0'

['use-repository-id']
type = 'cloned_named_arg'
long_name = 'TEST-use-repository-id'
source = 'repo'
"""
 When I execute dnf with args "install labirinto --TEST-use-repository-id=simple-base"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |


Scenario: I can define a new named argument to replace multiple options
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.0'

['list.all-available']
type = 'named_arg'
long_name = 'all-available'
attached_named_args = [
   { id_path = 'list.showduplicates' },
   { id_path = 'list.available' }
]
"""
  And I use repository "simple-updates"
  And I successfully execute dnf with args "install labirinto"
 When I execute dnf with args "list labirinto --all-available"
 Then the exit code is 0
  And stdout is
  """
  Available packages
  labirinto.src    1.0-1.fc29 simple-base
  labirinto.x86_64 1.0-1.fc29 simple-base
  labirinto.src    2.0-1.fc29 simple-updates
  labirinto.x86_64 2.0-1.fc29 simple-updates
  """


Scenario: Aliased command is printed to the user as part of the help
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.0'

['inthrone']
type = 'command'
attached_command = 'install'
descr = 'Install command test alias'
"""
 When I execute dnf with args "--help"
 Then stdout contains "inthrone"


Scenario: I can define a group for multiple commands or options
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.0'
['repo.test-query-aliases']
type = 'group'
header = 'Test Query Aliases:'

['repo.ls']
type = 'command'
attached_command = 'repo.list'
descr = "Alias for 'repo list'"
group_id = 'test-query-aliases'

['repo.if']
type = 'command'
attached_command = 'repo.info'
descr = "Alias for 'repo info'"
group_id = 'test-query-aliases'
"""
 When I execute dnf with args "repo --help"
 Then the exit code is 0
  And stdout matches line by line
"""
Usage:
  dnf5 \[GLOBAL OPTIONS\] repo <COMMAND> \.\.\.
\s+
Query Commands:
  list                          List repositories
  info                          Print details about repositories
\s+
Test Query Aliases:
  ls                            Alias for 'repo list'
  if                            Alias for 'repo info'
"""
