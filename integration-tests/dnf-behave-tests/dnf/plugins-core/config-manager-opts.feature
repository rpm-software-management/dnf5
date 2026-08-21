Feature: dnf "config-manager" command - test missing subcommand, "setopt" and "unsetopt" subcommands


Background:
  Given I create file "/etc/yum.repos.d/repo1.repo" with
      """
      [repo1]
      name=repository file
      enabled=1
      baseurl=http://something1.com/os/
      """
    And I create file "/etc/yum.repos.d/repo2.repo" with
      """
      [repo2]
      name=repository file
      enabled=0
      baseurl=http://something2.com/os/
      """
    And I create file "/etc/yum.repos.d/test_repo.repo" with
      """
      [test_repo]
      name=repository file
      enabled=1
      baseurl=http://something3.com/os/
      """


Scenario: when run "config-manager" without subcommand
   When I execute dnf with args "config-manager"
   Then the exit code is 2
    And stderr contains "Missing command"


Scenario: set new main option
  Given I create file "/etc/dnf/dnf.conf" with
      """
      [main]
      best=True
      """
   When I execute dnf with args "config-manager setopt skip_unavailable=1"
   Then the exit code is 0
    And file "/etc/dnf/dnf.conf" contents is
      """
      [main]
      best=True
      skip_unavailable=1
      """


Scenario: set main option multiple times with the same value is OK
  Given I create file "/etc/dnf/dnf.conf" with
      """
      [main]
      best=True
      """
   When I execute dnf with args "config-manager setopt skip_unavailable=1 best=False skip_unavailable=1"
   Then the exit code is 0
    And file "/etc/dnf/dnf.conf" contents is
      """
      [main]
      best=False
      skip_unavailable=1
      """


Scenario: set main option more times with different value
  Given I create file "/etc/dnf/dnf.conf" with
      """
      [main]
      best=True
      """
   When I execute dnf with args "config-manager setopt skip_unavailable=1 best=False skip_unavailable=0"
   Then the exit code is 1
    And stderr contains "Sets the "skip_unavailable" option again with a different value: "1" != "0""


Scenario: setting non-existent main option
  Given I create file "/etc/dnf/dnf.conf" with
      """
      [main]
      best=True
      """
   When I execute dnf with args "config-manager setopt nonexistent=1"
   Then the exit code is 1
    And stderr contains "Cannot set option: "nonexistent=1": Option "nonexistent" not found"


Scenario: main option - tests for setting an invalid value
  Given I create file "/etc/dnf/dnf.conf" with
      """
      [main]
      best=True
      """
   When I execute dnf with args "config-manager setopt best=XX"
   Then the exit code is 1
    And stderr contains "Cannot set option: "best=XX": Invalid boolean value "XX""


Scenario: change the value of main option
  Given I create file "/etc/dnf/dnf.conf" with
      """
      [main]
      best=True
      skip_unavailable=True
      """
   When I execute dnf with args "config-manager setopt best=False"
   Then the exit code is 0
    And file "/etc/dnf/dnf.conf" contents is
      """
      [main]
      best=False
      skip_unavailable=True
      """


Scenario: unset/remove an main option
  Given I create file "/etc/dnf/dnf.conf" with
      """
      [main]
      best=True
      skip_unavailable=True
      """
   When I execute dnf with args "config-manager unsetopt best"
   Then the exit code is 0
    And file "/etc/dnf/dnf.conf" contents is
      """
      [main]
      skip_unavailable=True
      """


Scenario: unset/remove unsupported main option is OK, but a warning is written
  Given I create file "/etc/dnf/dnf.conf" with
      """
      [main]
      best=True
      unsupported=1
      skip_unavailable=True
      """
   When I execute dnf with args "config-manager unsetopt best unsupported"
   Then the exit code is 0
    And file "/etc/dnf/dnf.conf" contents is
      """
      [main]
      skip_unavailable=True
      """
    And stderr is
      """
      config-manager: Request to remove unsupported main option: unsupported
      """


Scenario: unset/remove an main option, trying to unset an not set option is OK, but a warning is written
  Given I create file "/etc/dnf/dnf.conf" with
      """
      [main]
      best=True
      skip_unavailable=True
      """
   When I execute dnf with args "config-manager unsetopt best installroot"
   Then the exit code is 0
    And file "/etc/dnf/dnf.conf" contents is
      """
      [main]
      skip_unavailable=True
      """
    And stderr is
      """
      config-manager: Request to remove main option but it is not present in the config file: installroot
      """


Scenario: trying to unset an not set main option (config file is not found) is OK, but a warning is written
  Given I delete file "/etc/dnf/dnf.conf"
   When I execute dnf with args "config-manager unsetopt best"
   Then the exit code is 0
    And stderr is
      """
      config-manager: Request to remove main option but config file not found: {context.dnf.installroot}/etc/dnf/dnf.conf
      """


Scenario: repository configuration overrides - new option to "repo1"
  Given I create directory "/etc/dnf/repos.override.d/"
   When I execute dnf with args "config-manager setopt repo1.skip_if_unavailable=1"
   Then the exit code is 0
    And file "/etc/dnf/repos.override.d/99-config_manager.repo" contents is
      """
      # Generated by dnf5 config-manager.
      # Do not modify this file manually, use dnf5 config-manager instead.
      [repo1]
      skip_if_unavailable=1
      """


Scenario: repository configuration overrides - set option multiple times with the same value is OK
  Given I create directory "/etc/dnf/repos.override.d/"
   When I execute dnf with args "config-manager setopt repo1.skip_if_unavailable=1 repo1.priority=50 repo1.skip_if_unavailable=1"
   Then the exit code is 0
      """
      # Generated by dnf5 config-manager.
      # Do not modify this file manually, use dnf5 config-manager instead.
      [repo1]
      priority=50
      skip_if_unavailable=1
      """


Scenario: repository configuration overrides - set option more times with different value
  Given I create directory "/etc/dnf/repos.override.d/"
   When I execute dnf with args "config-manager setopt repo1.skip_if_unavailable=1 repo1.priority=50 repo1.skip_if_unavailable=0"
   Then the exit code is 1
    And stderr contains "Sets the "skip_if_unavailable" option of the repository "repo1" again with a different value: "1" != "0""


Scenario: setting non-existent repo option
  Given I create directory "/etc/dnf/repos.override.d/"
   When I execute dnf with args "config-manager setopt repo1.nonexistent=1"
   Then the exit code is 1
    And stderr contains "Cannot set repository option "repo1.nonexistent=1": Option "nonexistent" not found"


Scenario: repo option - tests for setting an invalid value
  Given I create directory "/etc/dnf/repos.override.d/"
   When I execute dnf with args "config-manager setopt repo1.skip_if_unavailable=XX"
   Then the exit code is 1
    And stderr contains "Cannot set repository option "repo1.skip_if_unavailable=XX": Invalid boolean value "XX""


Scenario: repository configuration overrides, "id" of non-existent repo
  Given I create directory "/etc/dnf/repos.override.d/"
   When I execute dnf with args "config-manager setopt non-exist-repo.skip_if_unavailable=1"
   Then the exit code is 1
    And stderr contains "No matching repository to modify: non-exist-repo"


Scenario: repository configuration overrides - new option, globs in repo id
  Given I create directory "/etc/dnf/repos.override.d/"
   When I execute dnf with args "config-manager setopt r?p*.skip_if_unavailable=1"
   Then the exit code is 0
    And file "/etc/dnf/repos.override.d/99-config_manager.repo" contents is
      """
      # Generated by dnf5 config-manager.
      # Do not modify this file manually, use dnf5 config-manager instead.
      [repo1]
      skip_if_unavailable=1
      [repo2]
      skip_if_unavailable=1
      """

Scenario: repository configuration overrides - globs in repo id, set option more times with different value
  Given I create directory "/etc/dnf/repos.override.d/"
   When I execute dnf with args "config-manager setopt r?p*.skip_if_unavailable=1 repo1.priority=50 repo2.skip_if_unavailable=0"
   Then the exit code is 1
    And stderr contains "Sets the "skip_if_unavailable" option of the repository "repo2" again with a different value: "1" != "0""


Scenario: repository configuration overrides, glob does not match any repo "id"
  Given I create directory "/etc/dnf/repos.override.d/"
   When I execute dnf with args "config-manager setopt non*repo.skip_if_unavailable=1"
   Then the exit code is 1
    And stderr contains "No matching repository to modify: non\*repo"


Scenario: repository configuration overrides - new option, missing destination directory
   When I execute dnf with args "config-manager setopt repo1.skip_if_unavailable=1"
   Then the exit code is 1
    And stderr contains "Directory ".*/etc/dnf/repos.override.d" does not exist. Add "--create-missing-dir" to create missing directories"


Scenario: repository configuration overrides - new option, "--create-missing-dir" creates missing destination directory
   When I execute dnf with args "config-manager setopt repo1.skip_if_unavailable=1 --create-missing-dir"
   Then the exit code is 0
    And file "/etc/dnf/repos.override.d/99-config_manager.repo" contents is
      """
      # Generated by dnf5 config-manager.
      # Do not modify this file manually, use dnf5 config-manager instead.
      [repo1]
      skip_if_unavailable=1
      """


Scenario: repository configuration overrides - new option, a file was found instead of destination directory
  Given I create file "/etc/dnf/repos.override.d" with
      """
      """
   When I execute dnf with args "config-manager setopt repo1.skip_if_unavailable=1"
   Then the exit code is 1
    And stderr contains "The path ".*/etc/dnf/repos.override.d" exists, but it is not a directory or a symlink to a directory"


Scenario: repository configuration overrides - new option, a symlink to non-existent object was found instead of destination directory
  Given I create symlink "/etc/dnf/repos.override.d" to file "/non-exist"
   When I execute dnf with args "config-manager setopt repo1.skip_if_unavailable=1"
   Then the exit code is 1
    And stderr contains "The path ".*/etc/dnf/repos.override.d" exists, but it is a symlink to a non-existent object"


Scenario: repository configuration overrides - unset/remove option
  Given I create file "/etc/dnf/repos.override.d/99-config_manager.repo" with
      """
      [repo1]
      enabled=0
      priority=40
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """
   When I execute dnf with args "config-manager unsetopt repo1.priority"
   Then the exit code is 0
    And file "/etc/dnf/repos.override.d/99-config_manager.repo" contents is
      """
      [repo1]
      enabled=0
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """


Scenario: repository configuration overrides - unset/remove option, globs in repo id
  Given I create file "/etc/dnf/repos.override.d/99-config_manager.repo" with
      """
      [repo1]
      enabled=0
      priority=40
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      [test_repo]
      enabled=1
      priotity=80
      skip_if_unavailable=0
      """
   When I execute dnf with args "config-manager unsetopt r*o?.priority"
   Then the exit code is 0
    And file "/etc/dnf/repos.override.d/99-config_manager.repo" contents is
      """
      [repo1]
      enabled=0
      skip_if_unavailable=1
      [repo2]
      enabled=1
      skip_if_unavailable=0
      [test_repo]
      enabled=1
      priotity=80
      skip_if_unavailable=0
      """


Scenario: repository configuration overrides - unset/remove unsupported option is OK, but a warning is written
  Given I create file "/etc/dnf/repos.override.d/99-config_manager.repo" with
      """
      [repo1]
      enabled=0
      unsupported=1
      priority=40
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """
   When I execute dnf with args "config-manager unsetopt repo1.unsupported repo1.priority"
   Then the exit code is 0
    And file "/etc/dnf/repos.override.d/99-config_manager.repo" contents is
      """
      [repo1]
      enabled=0
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """
    And stderr is
      """
      config-manager: Request to remove unsupported repository option: repo1.unsupported
      """


Scenario: repository configuration overrides - unset/remove option, repoid not in overrides is OK, but a warning is written
  Given I create file "/etc/dnf/repos.override.d/99-config_manager.repo" with
      """
      [repo1]
      enabled=0
      priority=40
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """
   When I execute dnf with args "config-manager unsetopt test_repo.priority repo1.priority"
   Then the exit code is 0
    And file "/etc/dnf/repos.override.d/99-config_manager.repo" contents is
      """
      [repo1]
      enabled=0
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """
    And stderr is
      """
      config-manager: Request to remove repository option but repoid is not present in the overrides: test_repo
      """


Scenario: repository configuration overrides - unset/remove option, trying to unset an not set option is OK, but a warning is written
  Given I create file "/etc/dnf/repos.override.d/99-config_manager.repo" with
      """
      [repo1]
      enabled=0
      priority=40
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """
   When I execute dnf with args "config-manager unsetopt repo1.cost repo1.priority"
   Then the exit code is 0
    And file "/etc/dnf/repos.override.d/99-config_manager.repo" contents is
      """
      [repo1]
      enabled=0
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """
    And stderr is
      """
      config-manager: Request to remove repository option but it is not present in the overrides: repo1.cost
      """


Scenario: repository configuration overrides - unset/remove option, no file with overrides found is OK, but a warning is written
   When I execute dnf with args "config-manager unsetopt repo1.cost"
   Then the exit code is 0
    And stderr is
      """
      config-manager: Request to remove repository option but file with overrides not found: {context.dnf.installroot}/etc/dnf/repos.override.d/99-config_manager.repo
      """


Scenario: repository configuration overrides - unset/remove option, empty section is removed
  Given I create file "/etc/dnf/repos.override.d/99-config_manager.repo" with
      """
      [repo1]
      enabled=0
      priority=40
      skip_if_unavailable=1
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """
   When I execute dnf with args "config-manager unsetopt repo1.enabled repo1.priority repo1.skip_if_unavailable"
   Then the exit code is 0
    And file "/etc/dnf/repos.override.d/99-config_manager.repo" contents is
      """
      [repo2]
      enabled=1
      priority=50
      skip_if_unavailable=0
      """
