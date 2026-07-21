#TODO(amatej): replace log checking with https://github.com/rpm-software-management/dnf5/issues/1335
Feature: Plugin enablement by config file and command line option


Background:
Given I do not disable plugins


Scenario: Verify that actions plugin is enabled
 When I execute dnf with args "repo list"
 Then file "/var/log/dnf5.log" contains lines
      """
      .* Loaded libdnf plugin "actions" .*
      """


Scenario: Disable enabled actions plugin from command line
 When I execute dnf with args "repo list --disable-plugin=actions"
 Then file "/var/log/dnf5.log" does not contain lines
      """
      .* Loaded libdnf plugin "actions" .*
      """


Scenario: Disable actions plugin by config file
Given I create file "/etc/dnf/plugins/actions.conf" with
      """
      [main]
      enabled= 0
      """
  And I configure dnf with
      | key            | value                                     |
      | pluginconfpath | {context.dnf.installroot}/etc/dnf/plugins |
 When I execute dnf with args "repo list"
 Then file "/var/log/dnf5.log" does not contain lines
      """
      .* Loaded libdnf plugin "actions" .*
      """


@bz1614539
Scenario: Enable disabled actions plugin from command line
Given I create file "/etc/dnf/plugins/actions.conf" with
      """
      [main]
      enabled= 0
      """
  And I configure dnf with
      | key            | value                                     |
      | pluginconfpath | {context.dnf.installroot}/etc/dnf/plugins |
 When I execute dnf with args "repo list --enable-plugin=actions"
 Then file "/var/log/dnf5.log" contains lines
      """
      .* Loaded libdnf plugin "actions" .*
      """
