# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Execute dnf shell commands from file


# Shell commands contained in the file:
#   repo enable dnf-ci-fedora
#   install flac
#   run
Background:
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |

Scenario: Execute dnf shell with file as argument
  When I execute dnf with args "shell {context.dnf.fixturesdir}/scripts/shell-commands"
  Then the exit code is 0
   And Transaction is following
       | Action        | Package                                   |
       | install       | flac-0:1.3.2-8.fc29.x86_64                |


Scenario: Execute dnf shell with file on input
  When I execute dnf with args "shell < {context.dnf.fixturesdir}/scripts/shell-commands"
  Then the exit code is 0
   And Transaction is following
       | Action        | Package                                   |
       | install       | flac-0:1.3.2-8.fc29.x86_64                |
