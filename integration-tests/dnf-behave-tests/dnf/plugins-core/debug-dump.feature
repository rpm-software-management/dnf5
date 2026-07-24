@xfail
# The plugin is missing: https://github.com/rpm-software-management/dnf5/issues/925
Feature: Test for debug plugin - dumping


Scenario: dnf debug-dump dumps file with configuration
  Given I use repository "debug-plugin"
    And I successfully execute dnf with args "install kernel-4.19.1 kernel-4.20.1"
   When I execute dnf with args "debug-dump {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And file "//{context.dnf.tempdir}/dump.txt" matches line by line
    """
    dnf-debug-dump version 1
    %%%%SYSTEM INFO
      uname: .*
      rpm ver: .*
      python ver: .*
    %%%%DNF INFO
      arch: .*
      basearch: .*
      releasever: .*
      dnf ver: .*
      enabled plugins: debug
      global excludes:
    %%%%RPMDB PROBLEMS
    %%%%RPMDB
      kernel-0:4\.19\.1-fc29\.x86_64
      kernel-0:4\.20\.1-fc29\.x86_64
    %%%%REPOS
    %debug-plugin - .*/fixtures/repos/debug-plugin
      excludes:
      kernel-0:4.18.1-fc29.src
      kernel-0:4.18.1-fc29.x86_64
      kernel-0:4.19.1-fc29.src
      kernel-0:4.19.1-fc29.x86_64
      kernel-0:4.20.1-fc29.src
      kernel-0:4.20.1-fc29.x86_64
      test-obsoleted-0:1-fc29.src
      test-obsoleted-0:1-fc29.x86_64
      test-obsoleter-0:2-fc29.src
      test-obsoleter-0:2-fc29.x86_64
      test-remove-0:1-fc29.src
      test-remove-0:1-fc29.x86_64
      test-replace-0:1-fc29.src
      test-replace-0:1-fc29.x86_64
      test-replace-0:2-fc29.src
      test-replace-0:2-fc29.x86_64
      test-replace-0:3-fc29.src
      test-replace-0:3-fc29.x86_64
    %%%%RPMDB VERSIONS
      all: [^ ]+
    """


Scenario: dnf debug-dump with --norepos skips dumping repositories contents
  Given I use repository "debug-plugin"
    And I successfully execute dnf with args "install kernel-4.19.1 kernel-4.20.1"
   When I execute dnf with args "debug-dump --norepos {context.dnf.tempdir}/dump.txt"
   Then the exit code is 0
    And file "//{context.dnf.tempdir}/dump.txt" matches line by line
    """
    dnf-debug-dump version 1
    %%%%SYSTEM INFO
      uname: .*
      rpm ver: .*
      python ver: .*
    %%%%DNF INFO
      arch: .*
      basearch: .*
      releasever: .*
      dnf ver: .*
      enabled plugins: debug
      global excludes:
    %%%%RPMDB PROBLEMS
    %%%%RPMDB
      kernel-0:4\.19\.1-fc29\.x86_64
      kernel-0:4\.20\.1-fc29\.x86_64
    %%%%RPMDB VERSIONS
      all: [^ ]+
    """
