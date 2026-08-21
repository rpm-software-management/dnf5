# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Module usage help

Scenario: I can print help using dnf module --help
 When I execute dnf with args "module --help"
 Then stdout contains "Usage:"
  And stdout contains "dnf5 \[GLOBAL OPTIONS\] module <COMMAND>"

Scenario: I can print help using dnf module -h
 When I execute dnf with args "module -h"
 Then stdout contains "Usage:"
  And stdout contains "dnf5 \[GLOBAL OPTIONS\] module <COMMAND>"
