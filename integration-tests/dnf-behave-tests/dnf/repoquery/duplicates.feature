Feature: Tests for the repoquery --duplicates functionality:


Scenario: show package installed in two versions
Given I use repository "repoquery-main"
  And I successfully execute rpm with args "-i --nodeps {context.scenario.repos_location}/repoquery-main/x86_64/bottom-a3-1.0-1.x86_64.rpm"
  And I successfully execute rpm with args "-i --nodeps {context.scenario.repos_location}/repoquery-main/x86_64/bottom-a3-2.0-1.x86_64.rpm"
 When I execute dnf with args "repoquery --duplicates"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom-a3-1:1.0-1.x86_64
      bottom-a3-1:2.0-1.x86_64
      """


Scenario: show package installed in two versions with other packages installed
Given I use repository "repoquery-main"
  And I execute dnf with args "install top-a-2.0-2"
  And I successfully execute rpm with args "-i --nodeps {context.scenario.repos_location}/repoquery-main/x86_64/bottom-a3-1.0-1.x86_64.rpm"
  And I successfully execute rpm with args "-i --nodeps {context.scenario.repos_location}/repoquery-main/x86_64/bottom-a3-2.0-1.x86_64.rpm"
 When I execute dnf with args "repoquery --duplicates"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom-a3-1:1.0-1.x86_64
      bottom-a3-1:2.0-1.x86_64
      """


Scenario: don't show installonly package installed in two versions
Given I use repository "repoquery-main"
  And I execute dnf with args "install top-a-2.0-2"
  And I successfully execute rpm with args "-i --nodeps {context.scenario.repos_location}/repoquery-main/x86_64/bottom-a3-1.0-1.x86_64.rpm"
  And I successfully execute rpm with args "-i --nodeps {context.scenario.repos_location}/repoquery-main/x86_64/bottom-a3-2.0-1.x86_64.rpm"
  And I configure dnf with
      | key                          | value      |
      | installonlypkgs              | bottom-a3  |
 When I execute dnf with args "repoquery --duplicates"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty

Scenario: don't resolve globs in installonlypkgs options
#  it is important to ensure the identical behavior with libsolv
Given I use repository "repoquery-main"
  And I execute dnf with args "install top-a-2.0-2"
  And I successfully execute rpm with args "-i --nodeps {context.scenario.repos_location}/repoquery-main/x86_64/bottom-a3-1.0-1.x86_64.rpm"
  And I successfully execute rpm with args "-i --nodeps {context.scenario.repos_location}/repoquery-main/x86_64/bottom-a3-2.0-1.x86_64.rpm"
  And I configure dnf with
      | key                          | value      |
      | installonlypkgs              | bottom-a3* |
 When I execute dnf with args "repoquery --duplicates"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom-a3-1:1.0-1.x86_64
      bottom-a3-1:2.0-1.x86_64
      """
