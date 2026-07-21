@xfail
# Reported as https://github.com/rpm-software-management/ci-dnf-stack/issues/1632
@no_installroot
Feature: Test execution of transaction() callback of removed plugin


Background: setup plugins and repositories
  Given I use repository "plugins-callbacks"
    And I configure dnf with
        | key          | value                                 |
        | pluginpath   | /dnf-plugins |
    And I do not disable plugins


Scenario: Transaction callback of removed plugin is not executed
  Given I use repository "simple-base"
    And I successfully execute dnf with args "install watermelon-1.0 labirinto"
        # watermelon plugin transaction() callback is executed when removing ordinary file
   When I execute dnf with args "remove -v labirinto"
   Then the exit code is 0
    And stdout contains "original watermelon-dnf-plugin transaction()"
        # when removing plugin package the transaction() callback is not executed
   When I execute dnf with args "remove -v watermelon"
   Then the exit code is 0
    And stdout does not contain "watermelon-dnf-plugin transaction()"


@bz1929163
Scenario: Removal of plugin in splitted package is detected
  Given I successfully execute dnf with args "install watermelon-1.0"
        # in version 2.0 the plugin is moved to separate subpackage which is not required
        # so effectively the plugin is removed
   When I execute dnf with args "upgrade watermelon-2.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | upgrade       | watermelon-0:2.0-1.fc29.x86_64        |
    And stdout does not contain "watermelon-dnf-plugin transaction()"


@bz1929163
Scenario: Moving plugin between subpackages is not considered removal of plugin
  Given I execute dnf with args "install watermelon-dnf-plugin-2.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                           |
        | install-dep   | watermelon-0:2.0-1.fc29.x86_64                    |
        | install       | watermelon-dnf-plugin-0:2.0-1.fc29.x86_64         |
        # in version 3.0 the plugin is moved back to main package and plugin package is obsoleted
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | watermelon-0:3.0-1.fc29.x86_64            |
        | obsoleted     | watermelon-dnf-plugin-0:2.0-1.fc29.x86_64 |
    And stdout contains "moved to subpackage watermelon-dnf-plugin transaction()"
