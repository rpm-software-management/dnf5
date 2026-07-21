Feature: Test for successful and failing rpm scriptlets


Background: Enable repository
  Given I use repository "scriptlets"
    # Some of the rpm scriptlet outputs can be quite long and since they are
    # truncated: https://github.com/rpm-software-management/dnf5/issues/1829
    # we need to force the width to see it in full.
    And I set environment variable "FORCE_COLUMNS" to "400"


# Disable on older fedoras and rhels because they don't contain rpm-6.0.91
@not.with_os=fedora__lt__45
@not.with_os=rhel__lt__11
Scenario Outline: Install a pkg with a successful scriptlet
   When I execute dnf with args "install <package>"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                  |
        | install       | <package>-0:1.0-1.x86_64 |
    And stderr contains "<output>"

Examples:
      | package              | output                                |
      | Package-pre-ok       | pre scriptlet successfully done       |
      | Package-pretrans-ok  | pretrans scriptlet successfully done  |
      | Package-post-ok      | post scriptlet successfully done      |
      | Package-posttrans-ok | posttrans scriptlet successfully done |


Scenario Outline: Install a pkg with a failing %pre[IN|TRANS] scriptlet
  When I execute dnf with args "install <package>"
  Then the exit code is 1
   And stderr contains ">>> Running <scriptlet> scriptlet: <package>"
   And stderr contains ">>> Error in <scriptlet> scriptlet: <package>"
   And stderr contains "Transaction failed: Rpm transaction failed."

Examples:
      | package                              | scriptlet       |
      | Package-pre-fail-0:1.0-1.x86_64      | %pre            |
      | Package-pretrans-fail-0:1.0-1.x86_64 | %pretrans       |


# Disable on older fedoras because they don't contain new rpm-6.0.0
@not.with_os=fedora__lt__43
Scenario Outline: Install a pkg with a failing %post[in|trans] scriptlet
  When I execute dnf with args "install <package>"
  Then the exit code is 1
   And Transaction is following
       | Action        | Package   |
       | install       | <package> |
   And stderr contains ">>> Running <scriptlet> scriptlet: <package>"
   And stderr contains ">>> Non-critical error in <scriptlet> scriptlet: <package>"

Examples:
      | package                               | scriptlet        |
      | Package-post-fail-0:1.0-1.x86_64      | %post            |
      | Package-posttrans-fail-0:1.0-1.x86_64 | %posttrans       |


# Disable on older fedoras and rhels because they don't contain rpm-6.0.91
@not.with_os=fedora__lt__45
@not.with_os=rhel__lt__11
Scenario Outline: Remove a pkg with a successful %[pre|post]un scriptlet
  When I execute dnf with args "install <package>"
  Then the exit code is 0
   And Transaction is following
       | Action        | Package                  |
       | install       | <package>-0:1.0-1.x86_64 |
  When I execute dnf with args "remove <package>"
  Then the exit code is 0
   And stderr contains "<output>"

Examples:
      | package           | output    |
      | Package-preun-ok  | preun scriptlet successfully done  |
      | Package-postun-ok | postun scriptlet successfully done |


Scenario: Remove a pkg with a failing %preun scriptlet
  When I execute dnf with args "install Package-preun-fail"
  Then the exit code is 0
   And Transaction is following
       | Action        | Package                           |
       | install       | Package-preun-fail-0:1.0-1.x86_64 |
  When I execute dnf with args "remove Package-preun-fail"
  Then the exit code is 1
   And stderr contains ">>> Running %preun scriptlet: Package-preun-fail-0:1.0-1.x86_64"
   And stderr contains ">>> Error in %preun scriptlet: Package-preun-fail-0:1.0-1.x86_64"
   And stderr contains "Transaction failed: Rpm transaction failed."
  When I execute dnf with args "--setopt=tsflags=noscripts remove Package-preun-fail"
  Then the exit code is 0
   And Transaction is following
       | Action        | Package                           |
       | remove        | Package-preun-fail-0:1.0-1.x86_64 |


# Disable on older fedoras because they don't contain new rpm-6.0.0
@not.with_os=fedora__lt__43
Scenario: Remove a pkg with a failing %postun scriptlet
  When I execute dnf with args "install Package-postun-fail"
  Then the exit code is 0
   And Transaction is following
       | Action        | Package                            |
       | install       | Package-postun-fail-0:1.0-1.x86_64 |
  When I execute dnf with args "remove Package-postun-fail"
  Then the exit code is 1
   And stderr contains ">>> Running %postun scriptlet: Package-postun-fail-0:1.0-1.x86_64"
   And stderr contains ">>> Non-critical error in %postun scriptlet: Package-postun-fail-0:1.0-1.x86_64"
   And stderr does not contain "Complete!"
   And Transaction is following
       | Action        | Package                           |
       | remove        | Package-postun-fail-0:1.0-1.x86_64 |


@bz1724779
# Disable on older fedoras and rhels because they don't contain rpm-6.0.91
@not.with_os=fedora__lt__45
@not.with_os=rhel__lt__11
Scenario: Output for triggered successful scriptlet of a package not present in transaction has temporarily just pkg name
 Given I successfully execute dnf with args "install Package-triggerin-ok"
  When I execute dnf with args "install Package-install-file"
  Then the exit code is 0
   And Transaction is following
       | Action        | Package                             |
       | install       | Package-install-file-0:1.0-1.x86_64 |
   And stderr contains "triggerin scriptlet \(Package-triggerin-ok\) for Package-install-file install/update successfully done"
   And stdout does not contain "Running scriptlet\s*:\s*Package-install-file"


# Disable on older fedoras because they don't contain new rpm-6.0.0
@not.with_os=fedora__lt__43
@bz1724779
Scenario: Correct output for triggered failed scriptlet of package not present in transaction
 Given I successfully execute dnf with args "install Package-triggerin-fail"
  When I execute dnf with args "install Package-install-file"
  Then the exit code is 1
   And stderr contains "failing on triggerin scriptlet"
   And stderr contains ">>> Running %triggerin scriptlet: Package-triggerin-fail-0:1.0-1.x86_64"
   And stderr contains ">>> Non-critical error in %triggerin scriptlet: Package-triggerin-fail-0:1.0-1.x86_64"
   And stderr does not contain "scriptlet: Package-install-file"


# Disable on older fedoras because they don't contain new rpm-6.0.0
@not.with_os=fedora__lt__43
@bz1724779
Scenario: Correct output for triggered failing transfiletriggerpostun scriptlet of package not present in transaction
 Given I successfully execute dnf with args "install Package-transfiletriggerpostun-fail"
   And I successfully execute dnf with args "install Package-install-file"
  When I execute dnf with args "remove Package-install-file"
  Then the exit code is 1
   And Transaction is following
       | Action       | Package                             |
       | remove       | Package-install-file-0:1.0-1.x86_64 |
   And stderr contains "transfiletriggerpostun scriptlet \(Package-transfiletriggerpostun-fail\) for uninstall transaction of Package-install-file is failing"
   And stderr contains ">>> Running %triggerpostun scriptlet: Package-transfiletriggerpostun-fail-0:1.0-1.x86_64"
   And stderr contains ">>> Non-critical error in %triggerpostun scriptlet: Package-transfiletriggerpostun-fail-0:1.0-1.x86_64"
   And stderr does not contain "scriptlet: Package-install-file"


# Disable on older fedoras and rhels because they don't contain rpm-6.0.91
@not.with_os=fedora__lt__45
@not.with_os=rhel__lt__11
@bz1724779
Scenario: Correct output for triggered successful file scriptlet of package not present in transaction
 Given I successfully execute dnf with args "install Package-filetriggerin-ok"
  When I execute dnf with args "install Package-install-file"
  Then the exit code is 0
   And Transaction is following
       | Action        | Package                             |
       | install       | Package-install-file-0:1.0-1.x86_64 |
   And stderr contains "filetriggerin scriptlet \(Package-filetriggerin-ok\) for Package-install-file install/update successfully done"
   And stdout does not contain "scriptlet: Package-install-file"


# Disable on older fedoras because they don't contain new rpm-6.0.0
@not.with_os=fedora__lt__43
@bz1724779
Scenario: Correct output for triggered failing file scriptlet of package not present in transaction
 Given I successfully execute dnf with args "install Package-filetriggerin-fail"
  When I execute dnf with args "install Package-install-file"
  Then the exit code is 1
   And stderr contains "filetriggerin scriptlet \(Package-filetriggerin-fail\) for Package-install-file install/update is failing"
   And stderr contains ">>> Running %triggerin scriptlet: Package-filetriggerin-fail-0:1.0-1.x86_64"
   And stderr contains ">>> Non-critical error in %triggerin scriptlet: Package-filetriggerin-fail-0:1.0-1.x86_64"
   And stderr does not contain "scriptlet: Package-install-file"


# Disable on older fedoras and rhels because they don't contain rpm-6.0.91
@not.with_os=fedora__lt__45
@not.with_os=rhel__lt__11
@bz1724779
Scenario: Correct output for triggered successful transfiletriggerpostun scriptlet of package not present in transaction
 Given I successfully execute dnf with args "install Package-transfiletriggerpostun-ok"
   And I successfully execute dnf with args "install Package-install-file"
  When I execute dnf with args "remove Package-install-file"
  Then the exit code is 0
   And Transaction is following
       | Action       | Package                             |
       | remove       | Package-install-file-0:1.0-1.x86_64 |
   And stderr contains "transfiletriggerpostun scriptlet \(Package-transfiletriggerpostun-ok\) for Package-install-file transaction uninstall successfully done"
   And stderr does not contain "scriptlet: Package-install-file"
