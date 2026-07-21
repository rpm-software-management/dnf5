Feature: Test error handling related to repositories

Scenario: Invalid character in ID in --repofrompath
 When I execute dnf with args "--repofrompath=a/b,URL list"
 Then the exit code is 1
  And stderr is
      """
      Invalid repository id "a/b": unexpected character '/'
      """


Scenario: Duplicate ID in --repofrompath
Given I use repository "simple-base"
 When I execute dnf with args "--repofrompath=simple-base,URL list"
 Then the exit code is 1
  And stderr is
      """
      Failed to create repo "simple-base": Id is present more than once in the configuration
      """


Scenario: --disablerepo=* does not disable --repofrompath
Given I use repository "simple-base"
 When I execute dnf with args "repo list --disablerepo=* --repofrompath=test,URL"
 Then the exit code is 0
  And stderr is empty
  And stdout is
      """
      repo id repo name
      test    test
      """


Scenario: Duplicate repo ID in repo configuration
Given I use repository "simple-base"
  And I create file "/{context.dnf.installroot}/etc/yum.repos.d/duplicate-id.repo" with
      """
      [simple-base]
      name = duplicate simple-base test repository
      enabled = 1
      baseurl = file:///duplicate/repo/id
      """
 When I execute dnf with args "repo list"
 Then the exit code is 0
  And stderr is empty
  And stdout is
      """
      repo id     repo name
      simple-base duplicate simple-base test repository
      """
  And file "/var/log/dnf5.log" contains lines
      """
      .*ERROR Failed to create repo "simple-base": Id is present more than once in the configuration
      """
