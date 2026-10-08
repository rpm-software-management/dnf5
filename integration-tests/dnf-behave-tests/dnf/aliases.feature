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


Scenario: command_flag alias maps a dnf4 flag to a subcommand
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['updateinfo.list']
type = 'command_flag'
attached_command = 'advisory.list'
"""
 When I execute dnf with args "updateinfo --list"
 Then the exit code is 0


Scenario: command_flag alias rewrites following values through a template
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['repoquery.whatneeds']
type = 'command_flag'
attached_command = 'repoquery'
positional_template = '--whatrequires=${}'
"""
 When I execute dnf with args "repoquery --whatneeds labirinto"
 Then the exit code is 0
  And stdout contains "vagare"


Scenario: command_flag alias requires config file version 1.2
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.1'

['updateinfo.list']
type = 'command_flag'
attached_command = 'advisory.list'
"""
 When I execute dnf with args "updateinfo --list"
 Then the exit code is 2
  And stderr contains "Used config file version \"1.1\" for alias type \"command_flag\""


Scenario: command_flag alias with reject_message prints guidance and keeps the parser error
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['repoquery.nogo']
type = 'command_flag'
reject_message = 'The nogo option has no equivalent'
"""
 When I execute dnf with args "repoquery --nogo"
 Then the exit code is 2
  And stderr contains "The nogo option has no equivalent"
  And stderr contains "Unknown argument \"--nogo\""


Scenario: command_flag alias with precedence picks the right entry when several flags match
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['updateinfo.list']
type = 'command_flag'
attached_command = 'advisory.list'
precedence = 10

['updateinfo.all']
type = 'command_flag'
attached_command = 'advisory.list'
"""
 When I execute dnf with args "updateinfo --all --list"
 Then the exit code is 0


Scenario: command_flag alias rewrites dashed tokens through a prefix map
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['repoquery.needsmap']
type = 'command_flag'
attached_command = 'repoquery'
token_prefix_maps = [{ prefix = '--need-', template = '--whatrequires=${}' }]
"""
 When I execute dnf with args "repoquery --needsmap --need-labirinto"
 Then the exit code is 0
  And stdout contains "vagare"


Scenario: command_flag alias leaves a value of a following option untouched
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['repoquery.whatneeds']
type = 'command_flag'
attached_command = 'repoquery'
positional_template = '--whatrequires=${}'
"""
 When I execute dnf with args "repoquery --whatneeds labirinto --exclude foo"
 Then the exit code is 0
  And stdout contains "vagare"


Scenario: command lines that match no command_flag alias are not touched
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['updateinfo.list']
type = 'command_flag'
attached_command = 'advisory.list'
"""
 When I execute dnf with args "repoquery --no-such-option"
 Then the exit code is 2
  And stderr contains "Unknown argument \"--no-such-option\""


Scenario: command_flag alias drops redundant bare operands with a note
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['repoquery.dropper']
type = 'command_flag'
attached_command = 'repoquery'
drop_bare_positionals = true
dropped_note = 'The dropper option takes no arguments'
"""
 When I execute dnf with args "repoquery --dropper labirinto"
 Then the exit code is 0
  And stderr contains "The dropper option takes no arguments: ignoring argument \"labirinto\""


Scenario: command_flag alias with a value gate prints guidance when values do not match
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['repoquery.gatedneeds']
type = 'command_flag'
attached_command = 'repoquery'
positional_template = '--whatrequires=${}'
positional_suffix_gate = '.rpm'
gate_reject_message = 'The gatedneeds option only translates .rpm values'
"""
 When I execute dnf with args "repoquery --gatedneeds labirinto"
 Then the exit code is 2
  And stderr contains "The gatedneeds option only translates .rpm values"


Scenario: command_flag alias naming an existing option is ignored and other entries still fire
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['repoquery.available']
type = 'command_flag'
attached_command = 'repoquery'

['repoquery.whatneeds']
type = 'command_flag'
attached_command = 'repoquery'
positional_template = '--whatrequires=${}'
"""
 When I execute dnf with args "repoquery --available --whatneeds labirinto"
 Then the exit code is 0
  And stdout contains "vagare"
  And stderr contains "Command flag alias \"repoquery.available\" matches an existing option of the command, ignored"


Scenario: command_flag alias keeps the typed command line in the log
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['updateinfo.list']
type = 'command_flag'
attached_command = 'advisory.list'
"""
 When I execute dnf with args "updateinfo --list"
 Then the exit code is 0
  And file "/var/log/dnf5.log" contains lines
"""
DNF5 launched with arguments: ".*updateinfo --list"
rewrote the command line to: .*advisory list
"""
  And file "/var/log/dnf5.log" does not contain lines
"""
DNF5 launched with arguments: ".*advisory list"
"""


Scenario: command_flag alias does not read a value of an attached command option as a trigger
Given I create directory "//etc/dnf/dnf5-aliases.d"
  And I create file "//etc/dnf/dnf5-aliases.d/TEST_ALIASES.conf" with
"""
version = '1.2'

['updateinfo.list']
type = 'command_flag'
attached_command = 'advisory.list'

['updateinfo.all']
type = 'command_flag'
attached_command = 'advisory.info'
precedence = 50
"""
 When I execute dnf with args "updateinfo --list --contains-pkgs --all"
 Then the exit code is 0
