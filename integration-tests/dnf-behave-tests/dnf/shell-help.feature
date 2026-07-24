# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Shell help


@bz1659328
Scenario: Using dnf shell, list available commands (dnf)
  Given I set dnf command to "dnf"
   When I open dnf shell session
    And I execute in dnf shell "help"
   Then stdout contains "usage: .+ \[options\] COMMAND"
    And stdout contains "List of Main Commands:"
    And stdout contains "alias\s+List or create command aliases"
    And stdout contains "General DNF options:"
    And stdout contains "-q, --quiet\s+quiet operation"
    And stdout contains "Shell specific arguments:"
    And stdout contains "repository \(or repo\)\s+enable, disable or list repositories"
      # "List of Plugin Commands:" depends on enabled plugins
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"

@bz1659328
Scenario: Using dnf shell, list available commands (yum)
  Given I set dnf command to "yum"
   When I open dnf shell session
    And I execute in dnf shell "help"
   Then stdout contains "usage: .+ \[options\] COMMAND"
    And stdout contains "List of Main Commands:"
    And stdout contains "alias\s+List or create command aliases"
    And stdout contains "General YUM options:"
    And stdout contains "-q, --quiet\s+quiet operation"
    And stdout contains "Shell specific arguments:"
    And stdout contains "repository \(or repo\)\s+enable, disable or list repositories"
      # "List of Plugin Commands:" depends on enabled plugins
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"
