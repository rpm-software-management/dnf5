Feature: history list

Background:
Given I use repository "dnf-ci-fedora"
  # create some history to start with
  And I successfully execute dnf with args "install abcde basesystem"
  And I successfully execute dnf with args "remove abcde"
  And I successfully execute dnf with args "install nodejs"


Scenario: history list
 When I execute dnf with args "history list"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 3  |         |         | 5       |
      | 2  |         |         | 3       |
      | 1  |         |         | 6       |


Scenario: history without sub-command
 When I execute dnf with args "history"
 Then the exit code is 2
  And stdout is empty
  And stderr is
  """
  Missing command. Add "--help" for more information about the arguments.
  """


# single item tests
Scenario: history list 2
 When I execute dnf with args "history list 2"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 2  |         |         | 3       |


Scenario: history list with mulitple args
 When I execute dnf with args "history list 1 2 3"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 3  |         |         | 5       |
      | 2  |         |         | 3       |
      | 1  |         |         | 6       |


Scenario: history list last
 When I execute dnf with args "history list last"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 3  |         |         | 5       |


Scenario: history last without subcommand
 When I execute dnf with args "history last"
 Then the exit code is 2
  And stdout is empty
  And stderr is
  """
  Unknown argument "last" for command "history". Add "--help" for more information about the arguments.
  """


Scenario: history list last-1
 When I execute dnf with args "history list last-1"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 2  |         |         | 3       |


# range tests
Scenario: history list 1..last-1
 When I execute dnf with args "history list 1..last-1"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 2  |         |         | 3       |
      | 1  |         |         | 6       |


Scenario: history list 1..last-2
 When I execute dnf with args "history list 1..last-2"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 1  |         |         | 6       |


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1656
Scenario: history list 1..-1
 When I execute dnf with args "history 1..-1"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 2  |         | Removed | 3       |
      | 1  |         | Install | 6       |


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1656
Scenario: history list 1..-2
 When I execute dnf with args "history 1..-2"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 1  |         | Install | 6       |


Scenario: history list 2..3
 When I execute dnf with args "history list 2..3"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 3  |         |         | 5       |
      | 2  |         |         | 3       |


Scenario: history list 10..11
 When I execute dnf with args "history list 10..11"
 Then the exit code is 0
  And stdout is empty


Scenario: history list last..11
 When I execute dnf with args "history list last..11"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 3  |         |         | 5       |


# "invalid" range tests
Scenario: history list 3..2
 When I execute dnf with args "history list 3..2"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 3  |         |         | 5       |
      | 2  |         |         | 3       |


Scenario: history list last-1..1
 When I execute dnf with args "history list last-1..1"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 2  |         |         | 3       |
      | 1  |         |         | 6       |


Scenario: history list 11..last-1
 When I execute dnf with args "history list 11..last-1"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 3  |         |         | 5       |
      | 2  |         |         | 3       |


Scenario: history list last-1..aaa
 When I execute dnf with args "history list last-1..aaa"
 Then the exit code is 1
  And stdout is empty
  And stderr is
      """
      Invalid transaction ID range "last-1..aaa", "ID" or "ID..ID" expected, where ID is "NUMBER", "last" or "last-NUMBER".
      """


Scenario: history list 12a..bc
 When I execute dnf with args "history list 12a..bc"
 Then the exit code is 1
  And stdout is empty
  And stderr is
      """
      Invalid transaction ID range "12a..bc", "ID" or "ID..ID" expected, where ID is "NUMBER", "last" or "last-NUMBER".
      """


# package name tests
Scenario: history abcde
 When I execute dnf with args "history list --contains-pkgs=abcde"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 2  |         |         | 3       |
      | 1  |         |         | 6       |


Scenario: history filesystem
 When I execute dnf with args "history list --contains-pkgs=filesystem"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 1  |         |         | 6       |


Scenario: history lame (no transaction with such package)
 When I execute dnf with args "history list --contains-pkgs=lame"
 Then the exit code is 0
  And stdout is empty


# Reported as https://github.com/rpm-software-management/dnf5/issues/2025
@bz1786335
@bz1786316
@bz1852577
@bz1906970
Scenario: history table width doesn't exceed 80 characters when there is no terminal
 When I execute dnf with args "history list | head -1"
 Then the exit code is 0
  And stdout matches line by line
  """
  ID Command line {1,28}Date and time       Action\(s\) Altered
  """


@bz1846692
Scenario: history list --reverse
 When I execute dnf with args "history list --reverse"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 1  |         |         | 6       |
      | 2  |         |         | 3       |
      | 3  |         |         | 5       |


@bz1846692
Scenario: history 2..3 --reverse
 When I execute dnf with args "history list 2..3 --reverse"
 Then the exit code is 0
  And stdout is history list
      | Id | Command | Action  | Altered |
      | 2  |         |         | 3       |
      | 3  |         |         | 5       |
