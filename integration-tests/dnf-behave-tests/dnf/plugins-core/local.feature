# Local plugin is not built on RHEL >= 10.
@not.with_os=rhel__ge__10
Feature: Tests for local plugin


Background:
Given I enable plugin "local"
  And I configure dnf with
    | key            | value                                              |
    | pluginconfpath | {context.dnf.installroot}/etc/dnf/libdnf5-plugins  |
  And I create and substitute file "/etc/dnf/libdnf5-plugins/local.conf" with
   """
   [main]
   name = local
   enabled = 1
   repodir = {context.dnf.installroot}/repo
   [createrepo]
   enabled = 1
   """


Scenario: installed package is added to the local repo with no createrepo_c output
Given I use repository "simple-base"
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
 When I execute dnf with args "rq --repo _dnf_local"
 Then the exit code is 0
  And stdout is
   """
   labirinto-0:1.0-1.fc29.x86_64
   """
  And stdout does not contain "local plugin: createrepo_c: Directory walk started"


Scenario: local repo is preferred because it has lower than default cost
Given I use repository "simple-base" with configuration
      | key          | value                                                                      |
      | enabled      | 1                                                                          |
  And I successfully execute dnf with args "install dedalo"
  And I successfully execute dnf with args "remove dedalo"
 When I execute dnf with args "install dedalo"
 Then the exit code is 0
  And dnf5 transaction items for transaction "last" are
      | action         | package                    | reason | repository |
      | Install        | dedalo-0:1.0-1.fc29.x86_64 | User   | _dnf_local |


Scenario: local repo is not cached
Given I use repository "simple-base"
# local repo doesn't exist yet, this dnf run creates it
  And I successfully execute dnf with args "install labirinto"
# local repo exists and it is loaded by this run
 When I execute dnf with args "rq --repo _dnf_local"
 Then the exit code is 0
  And stdout is
   """
   labirinto-0:1.0-1.fc29.x86_64
   """
# add a new package to the local repo, this generates new metadata
 When I execute dnf with args "install vagare"
 Then the exit code is 0
# new metadata are loaded even without --refresh
 When I execute dnf with args "rq --repo _dnf_local"
 Then the exit code is 0
  And stdout is
   """
   labirinto-0:1.0-1.fc29.x86_64
   vagare-0:1.0-1.fc29.x86_64
   """


Scenario: with verbose enabled stdout contains both regular and debug createrepo_c output
Given I create and substitute file "/etc/dnf/libdnf5-plugins/local.conf" with
   """
   [main]
   name = local
   enabled = 1
   repodir = {context.dnf.installroot}/repo
   [createrepo]
   enabled = 1
   verbose = true
   """
  And I use repository "simple-base"
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
  And stdout contains lines matching
   """
   local plugin: createrepo_c: \d\d:\d\d:\d\d: Thread pool ready
   local plugin: createrepo_c: \d\d:\d\d:\d\d: Package count: 1
   local plugin: createrepo_c: \d\d:\d\d:\d\d: Generating repomd.xml
   local plugin: createrepo_c: \d\d:\d\d:\d\d: All done
   local plugin: createrepo_c: Directory walk started
   local plugin: createrepo_c: Pool finished
   """
