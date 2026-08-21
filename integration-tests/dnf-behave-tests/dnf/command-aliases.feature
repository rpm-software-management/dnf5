Feature: Tests for command aliases availability


Scenario: "nonexistent" is not an alias for any dnf command
   When I execute dnf with args "nonexistent -h"
   Then the exit code is 2
   And stderr is
   """
   Unknown argument "nonexistent" for command "dnf5".
   It could be a command provided by a plugin, try: dnf5 install 'dnf5-command(nonexistent)'
   """


Scenario Outline: "<alias>" is an alias for "<command>"
   When I execute dnf with args "<alias> -h"
   Then the exit code is 0
    # The exact output is now under discussion
    # And stdout contains "Usage:\n.*<command>"
    And stdout contains "Usage:\n"
    And stdout does not contain "Unknown argument"
    And stderr does not contain "Unknown argument"

Examples:
        | command             | alias                        |
        | advisory            | updateinfo                   |
        | autoremove          | autoremove                   |
        | check-upgrade       | check-update                 |
        | check-upgrade       | check-upgrade                |
        | clean               | clean                        |
        | distro-sync         | distro-sync                  |
        | downgrade           | dg                           |
        | download            | download                     |
        | downgrade           | downgrade                    |
        | environment         | environment                  |
        | group               | group                        |
        | group               | grp                          |
        | history             | history                      |
        | info                | info                         |
        | install             | in                           |
        | install             | install                      |
        | leaves              | leaves                       |
        | list                | list                         |
        | list                | ls                           |
        | makecache           | mc                           |
        | mark                | mark                         |
        | reinstall           | rei                          |
        | reinstall           | reinstall                    |
        | remove              | remove                       |
        | remove              | rm                           |
        | repo info           | repoinfo                     |
        | repo list           | repolist                     |
        | repoquery           | repoquery                    |
        | repoquery           | rq                           |
        | search              | search                       |
        | swap                | swap                         |
        | upgrade             | up                           |
        | upgrade             | update                       |
        | upgrade             | upgrade                      |
        | upgrade             | upgrade-minimal              |

# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Examples:
        | command             | alias                        |
        | module              | module                       |
