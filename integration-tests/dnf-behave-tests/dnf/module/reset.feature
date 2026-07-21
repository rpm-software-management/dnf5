# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Reset modules


Background:
Given I use repository "dnf-ci-fedora-modular"


@bz1827424
Scenario: I can reset a disabled default stream back to its default state
 When I execute dnf with args "module disable nodejs"
 Then the exit code is 0
  And modules state is following
      | Module | State    | Stream | Profiles |
      | nodejs | disabled |        |          |
 When I execute dnf with args "module list"
 Then module list contains
      | Name   | Stream   | Profiles                          |
      | nodejs | 8 [d][x] | default [d], development, minimal |
      | nodejs | 10 [x]   | default [d], development, minimal |
      | nodejs | 11 [x]   | default, development, minimal     |
 When I execute dnf with args "module reset nodejs"
 Then the exit code is 0
  And stdout contains "Resetting modules:"
  And stdout contains "nodejs"
  And modules state is following
      | Module | State     | Stream | Profiles |
      | nodejs | available |        |          |
 When I execute dnf with args "module list nodejs"
  Then module list contains
      | Name   | Stream | Profiles                          |
      | nodejs | 8 [d]  | default [d], development, minimal |
      | nodejs | 10     | default [d], development, minimal |
      | nodejs | 11     | default, development, minimal     |


@bz1827424
Scenario: I can reset a disabled non-default stream back to a non-default state
 When I execute dnf with args "module disable dwm"
 Then the exit code is 0
  And modules state is following
      | Module | State    | Stream | Profiles |
      | dwm    | disabled |        |          |
 When I execute dnf with args "module list"
 Then module list contains
      | Name | Stream   | Profiles |
      | dwm  | 6.0 [x]  | default  |
 When I execute dnf with args "module reset dwm"
 Then the exit code is 0
  And stdout contains "Resetting modules:"
  And stdout contains "dwm"
  And modules state is following
      | Module | State     | Stream | Profiles |
      | dwm    | available |        |          |
 When I execute dnf with args "module list"
 Then module list contains
      | Name | Stream | Profiles |
      | dwm  | 6.0    | default  |


@bz1827424
Scenario: Resetting of a default stream does nothing
 When I execute dnf with args "module list nodejs"
 Then module list contains
      | Name   | Stream | Profiles                          |
      | nodejs | 8 [d]  | default [d], development, minimal |
      | nodejs | 10     | default [d], development, minimal |
      | nodejs | 11     | default, development, minimal     |
 When I execute dnf with args "module reset nodejs"
 Then the exit code is 0
  And stdout contains "Nothing to do"
  And modules state is following
      | Module | State | Stream | Profiles |
      | nodejs |       |        |          |
 When I execute dnf with args "module list nodejs"
 Then module list contains
      | Name   | Stream | Profiles                          |
      | nodejs | 8 [d]  | default [d], development, minimal |
      | nodejs | 10     | default [d], development, minimal |
      | nodejs | 11     | default, development, minimal     |


@bz1827424
Scenario: Resetting of a non-default non-enabled stream does nothing
 When I execute dnf with args "module list dwm"
 Then module list contains
      | Name | Stream | Profiles |
      | dwm  | 6.0    | default  |
 Then I execute dnf with args "module reset dwm"
 Then the exit code is 0
  And stdout contains "Nothing to do"
  And modules state is following
      | Module | State | Stream | Profiles |
      | dwm    |       |        |          |
 When I execute dnf with args "module list dwm"
 Then module list contains
      | Name | Stream | Profiles |
      | dwm  | 6.0    | default  |


@bz1827424
@bz1677640
Scenario: I can reset an enabled default stream back to its non-enabled default state
 When I execute dnf with args "module enable nodejs:8"
 Then the exit code is 0
  And modules state is following
      | Module | State   | Stream | Profiles |
      | nodejs | enabled | 8      |          |
 When I execute dnf with args "module list nodejs"
 Then module list contains
      | Name   | Stream   | Profiles                          |
      | nodejs | 8 [d][e] | default [d], development, minimal |
      | nodejs | 10       | default [d], development, minimal |
      | nodejs | 11       | default, development, minimal     |
 When I execute dnf with args "module reset nodejs"
 Then the exit code is 0
  And stdout contains "Resetting modules:"
  And stdout contains "nodejs"
  And modules state is following
      | Module | State     | Stream | Profiles |
      | nodejs | available |        |          |
 When I execute dnf with args "module list nodejs"
 Then module list contains
      | Name   | Stream | Profiles                          |
      | nodejs | 8 [d]  | default [d], development, minimal |
      | nodejs | 10     | default [d], development, minimal |
      | nodejs | 11     | default, development, minimal     |


@bz1827424
Scenario: I can reset an enabled non-default stream back to a non-enabled state
 When I execute dnf with args "module enable dwm:6.0/default"
 Then the exit code is 0
  And modules state is following
      | Module | State   | Stream | Profiles |
      | dwm    | enabled | 6.0    |          |
 When I execute dnf with args "module list dwm"
 Then module list contains
      | Name | Stream  | Profiles |
      | dwm  | 6.0 [e] | default  |
 When I execute dnf with args "module reset dwm"
 Then the exit code is 0
  And stdout contains "Resetting modules:"
  And stdout contains "dwm"
  And modules state is following
      | Module | State     | Stream | Profiles |
      | dwm    | available |        |          |
 When I execute dnf with args "module list dwm"
 Then module list contains
      | Name | Stream | Profiles |
      | dwm  | 6.0    | default  |
