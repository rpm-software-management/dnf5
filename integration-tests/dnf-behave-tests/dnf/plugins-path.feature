#TODO(amatej): replace log checking with https://github.com/rpm-software-management/dnf5/issues/1335
Feature: Pluginspath and pluginsconfpath test


Background:
Given I do not disable plugins


Scenario: Redirect pluginpath and pluginconfpath
Given I successfully execute dnf with args "repo list"
  And file "/var/log/dnf5.log" contains lines
      """
      .* Loaded libdnf plugin "actions" .*
      """
  And I configure dnf with
      | key        | value                                  |
      | pluginpath | {context.dnf.installroot}/test/plugins |
 When I execute dnf with args "repo list"
 Then the exit code is 1
  And stdout is empty
  And stderr is
      """
      Cannot load libdnf plugin enabled from: /etc/dnf/libdnf5-plugins/actions.conf
       Cannot find plugin library "{context.dnf.installroot}/test/plugins/actions.so"
      """
# Remove the previous log which contains successfully loaded plugin actions msg
Given I delete file "/var/log/dnf5.log"
  And I configure dnf with
      | key            | value                                  |
      | pluginconfpath | {context.dnf.installroot}/test/plugins |
 When I execute dnf with args "repo list"
 Then the exit code is 0
  And stdout is empty
  And stderr is empty
  And file "/var/log/dnf5.log" does not contain lines
      """
      .* Loaded libdnf plugin "actions" .*
      """


Scenario: Test default pluginsconfpath
Given I successfully execute dnf with args "repo list"
  And file "/var/log/dnf5.log" contains lines
      """
      .* Loaded libdnf plugin "actions" .*
      """
  # create actions.conf inside the installroot
  And I create file "/etc/dnf/libdnf5-plugins/actions.conf" with
      """
      [main]
      enabled = 0
      """
 # pluginconfpath is not related to installroot, so actions are not disabled
 When I execute dnf with args "repo list"
 Then the exit code is 0
  And stdout is empty
  And stderr is empty
  And file "/var/log/dnf5.log" contains lines
      """
      .* Loaded libdnf plugin "actions" .*
      """


Scenario: Redirect pluginsconfpath in dnf.conf
Given I successfully execute dnf with args "repo list"
  And file "/var/log/dnf5.log" contains lines
      """
      .* Loaded libdnf plugin "actions" .*
      """
  And I configure dnf with
      | key            | value                                         |
      | pluginconfpath | {context.dnf.installroot}/test/pluginconfpath |
  And I create file "/test/pluginconfpath/actions.conf" with
      """
      [main]
      enabled = 0
      """
  # Remove the previous log which contains successfully loaded plugin actions msg
  And I delete file "/var/log/dnf5.log"
 # pluginconfpath is now set in installroot, so versionlock is disabled
 When I execute dnf with args "repo list"
 Then the exit code is 0
  And stdout is empty
  And stderr is empty
  And file "/var/log/dnf5.log" does not contain lines
      """
      .* Loaded libdnf plugin "actions" .*
      """
