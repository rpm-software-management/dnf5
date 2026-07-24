@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1809
Feature: Tests for enabling plugins containing dashes in names

Background: Enable plugins
  Given I do not disable plugins
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "repoquery empty --verbose"
   Then the exit code is 0
    And stdout contains "Loaded plugins: .*config-manager"

@bz1980712
Scenario: Existing plugin containing dashes in the name could be disabled
   When I execute dnf with args "repoquery empty --disableplugin=config-manager --verbose"
   Then the exit code is 0
    And stdout does not contain "Loaded plugins: .*config-manager"
    And stderr does not contain "No matches found for the following disable plugin patterns: config-manager"

@bz1980712
Scenario: Existing plugin containing dashes in the name could be enabled
   When I execute dnf with args "repoquery empty --disableplugin='*' --enableplugin=config-manager --verbose"
   Then the exit code is 0
    And stdout contains "Loaded plugins: .*config-manager"
    And stderr does not contain "No matches found for the following enable plugin patterns: config-manager"

@bz1980712
Scenario: Existing plugin containing dashes in the name could be disabled using the underscored name
   When I execute dnf with args "repoquery empty --disableplugin=config_manager --verbose"
   Then the exit code is 0
    And stdout does not contain "Loaded plugins: .*config-manager"
    And stderr does not contain "No matches found for the following disable plugin patterns: config_manager"

@bz1980712
Scenario: Existing plugin containing dashes in the name could be enabled using the underscored name
   When I execute dnf with args "repoquery empty --disableplugin='*' --enableplugin=config_manager --verbose"
   Then the exit code is 0
    And stdout contains "Loaded plugins: .*config-manager"
    And stderr does not contain "No matches found for the following enable plugin patterns: config_manager"
