# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: repoquery tests for handling modularity.

Background:
Given I use repository "repoquery-modules"


Scenario: Test --disable-modular-filtering with a default module stream
 When I execute dnf with args "repoquery nodejs"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
      """
 When I execute dnf with args "repoquery nodejs --disable-modular-filtering"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      nodejs-1:10.0.0-1.src
      nodejs-1:10.0.0-1.x86_64
      nodejs-1:5.3.1-1.module_2011+41787af0.src
      nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
      nodejs-1:8.11.4-1.module_2030+42747d40.src
      nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
      """

Scenario: Test --disable-modular-filtering with an enabled module stream
 When I execute dnf with args "module enable nodejs:5"
 When I execute dnf with args "repoquery nodejs"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
      """
 When I execute dnf with args "repoquery nodejs --disable-modular-filtering"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      nodejs-1:10.0.0-1.src
      nodejs-1:10.0.0-1.x86_64
      nodejs-1:5.3.1-1.module_2011+41787af0.src
      nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
      nodejs-1:8.11.4-1.module_2030+42747d40.src
      nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
      """
