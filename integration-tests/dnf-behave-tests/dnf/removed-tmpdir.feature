Feature: Better error message when TMPDIR is missing

@bz2019993
Scenario: show detailed error message when $TMPDIR does not exist
Given I use repository "dnf-ci-fedora" with configuration
      | key             | value                                       |
      | baseurl         |                                             |
      | mirrorlist      | {context.dnf.installroot}/tmp/mirrorlist    |
      | gpgcheck        | 0                                           |
  And I set environment variable "TMPDIR" to "/tmp/dummy.ABC123"
 When I execute dnf with args "repoquery empty --refresh"
 Then the exit code is 1
  And stdout is empty
  And stderr contains "Cannot create temporary file - mkstemp '/tmp/dummy.ABC123/librepo-tmp-*"
