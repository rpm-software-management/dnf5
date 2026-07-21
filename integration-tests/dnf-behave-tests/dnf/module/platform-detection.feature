# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
@bz1688462
Feature: Detecting proper modular platform

Background:
  Given I use repository "dnf-ci-pseudo-platform-modular"


Scenario: Platform pseudo module name:stream is created based on /usr/lib/os-release
  Given I do not set default module platformid
    And I create file "/usr/lib/os-release" with
        """
        NAME=PsedoDistro
        VERSION="6 (dwm-team)"
        ID=pseudo
        VERSION_ID=6
        PLATFORM_ID="pseudoplatform:6.0"
        PRETTY_NAME="PseudoDistro 6 (dwm-team)"
        """
   When I execute dnf with args "module enable dwm:6.0"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | dwm       | enabled   | 6.0       |           |


@jiraRHELPLAN-6083
Scenario: Platform pseudo module name:stream is created based on /etc/os-release
  Given I do not set default module platformid
    And I create file "/usr/lib/os-release" with
        """
        NAME=PsedoDistro
        VERSION="6 (dwm-team)"
        ID=pseudo
        VERSION_ID=6
        PLATFORM_ID="not_to_be_used_pseudoplatform:6.0"
        PRETTY_NAME="PseudoDistro 6 (dwm-team)"
        """
    And I create file "/etc/os-release" with
        """
        NAME=PsedoDistro
        VERSION="6 (dwm-team)"
        ID=pseudo
        VERSION_ID=6
        PLATFORM_ID="pseudoplatform:6.0"
        PRETTY_NAME="PseudoDistro 6 (dwm-team)"
        """
   When I execute dnf with args "module enable dwm:6.0"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | dwm       | enabled   | 6.0       |           |



Scenario: Platform is detected using virtual provide of installed os-release package
  Given I do not set default module platformid
    And I create file "/etc/os-release" with
        """
        NAME=PsedoDistro
        VERSION="6 (dwm-team)"
        ID=pseudo
        VERSION_ID=6
        PLATFORM_ID="not_to_be_used_pseudoplatform:6.0"
        PRETTY_NAME="PseudoDistro 6 (dwm-team)"
        """
   When I execute dnf with args "install {context.dnf.fixturesdir}/repos/dnf-ci-pseudo-platform/noarch/fedora-release-29-1.noarch.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | fedora-release-0:29-1.noarch          |
   When I execute dnf with args "module enable dwm:6.0"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | dwm       | enabled   | 6.0       |           |

@bz1709453
Scenario: Platform is detected using virtual provide of os-release package in enabled repo but not from excluded package
  Given I do not set default module platformid
    And I use repository "dnf-ci-pseudo-platform"
    And I use repository "module-platform-detection-excluded" with configuration
        | key         | value |
        | includepkgs | xget  |
    And I create file "/etc/os-release" with
        """
        NAME=PsedoDistro
        VERSION="6 (dwm-team)"
        ID=pseudo
        VERSION_ID=6
        PLATFORM_ID="not_to_be_used_pseudoplatform:6.0"
        PRETTY_NAME="PseudoDistro 6 (dwm-team)"
        """
   When I execute dnf with args "module enable dwm:6.0"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | dwm       | enabled   | 6.0       |           |


Scenario: Platform is detected from command line --setopt option
  Given I use repository "dnf-ci-pseudo-platform"
    And I set default module platformid to "cmdline_pseudoplatform:6.0"
   When I execute dnf with args "module enable dwm:6.0"
   Then the exit code is 1
    And stderr contains "nothing provides module\(pseudoplatform:6\.0\) needed by module dwm"
  Given I set default module platformid to "pseudoplatform:6.0"
   When I execute dnf with args "module enable dwm:6.0"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | dwm       | enabled   | 6.0       |           |
