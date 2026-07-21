@not.with_dnf=4
Feature: Tests for actions plugin

Background:
  Given I enable plugin "actions"
    And I configure dnf with
      | key            | value                                              |
      | pluginconfpath | {context.dnf.installroot}/etc/dnf/libdnf5-plugins  |
    And I create and substitute file "/etc/dnf/libdnf5-plugins/actions.conf" with
    """
    [main]
    enabled = 1
    """
    And I use repository "dnf-ci-fedora" with configuration
      | key     | value                            |
      | name    | DNF CI fedora base repository    |
      | enabled | 1                                |
    And I use repository "dnf-ci-fedora-updates" with configuration
      | key     | value                            |
      | name    | DNF CI fedora updates repository |
      | enabled | 0                                |
    And I use repository "dnf-ci-thirdparty" with configuration
      | key     | value                            |
      | name    | DNF CI thirdparty repo, test     |
      | enabled | 0                                |


Scenario: Test substitutions and settings of libdnf variables, configuration options and actions plugin temporary variables
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    # Print the value of the configuration option "defaultyes"
    pre_base_setup::::/bin/sh -c echo\ 'pre_base_setup:\ conf.defaultyes=${{conf.defaultyes}}'\ >>\ {context.dnf.installroot}/actions.log

    # A substitution will take place. The resulting action is the same as the previous one, it is filtered out.
    pre_base_setup::::/bin/sh -c echo\ 'pre_base_setup:\ conf.defaultyes=${{conf.defaultyes}}'\ >>\ {context.dnf.installroot}/actions.log

    # Create libdnf variable "test1" with value "Value1",
    # set configuration option "defaultyes" to 'true',
    # create temporary actions plugin variable "test_variable" with value "Value2"
    pre_base_setup::::/bin/sh -c echo\ -e\ "var.test1=Value1\\nconf.defaultyes=true\\ntmp.test_variable=Value2"

    # Print value of: libdnf variable "test1", configuration option "defaultyes", plugin temporary variable "test_variable"
    post_base_setup::::/bin/sh -c echo\ 'post_base_setup:\ var.test1=${{var.test1}}\ conf.defaultyes=${{conf.defaultyes}}\ tmp.test_variable=${{tmp.test_variable}}'\ >>\ {context.dnf.installroot}/actions.log

    # Delete temporary plugin variable "test_variable"
    post_base_setup::::/bin/sh -c echo\ -e\ "tmp.test_variable"

    # Nothing will be done. We cannot print the value of the plugin's temporary variable "test_variable" because it does not exist - it was deleted by a previous action.
    post_base_setup::::/bin/sh -c echo\ 'post_base_setup:\ tmp.test_variable=${{tmp.test_variable}}'\ >>\ {context.dnf.installroot}/actions.log

    # Print the value of the libdnf variable "releasever"
    post_base_setup::::/bin/sh -c echo\ 'post_base_setup:\ var.releasever=${{var.releasever}}'\ >>\ {context.dnf.installroot}/actions.log

    # Create temporary actions plugin variable "test_variable" with value "Value_set_in_pre_transaction"
    pre_transaction::::/bin/sh -c echo\ -e\ "tmp.test_variable=Value_set_in_pre_transaction"

    # Print a line for each package in the transaction - before executing the transaction
    pre_transaction:*:::/bin/sh -c echo\ 'pre_transaction:\ ${{pkg.action}}\ ${{pkg.name}}-${{pkg.epoch}}:${{pkg.version}}-${{pkg.release}}.${{pkg.arch}}\ repo\ ${{pkg.repo_id}}'\ >>\ {context.dnf.installroot}/actions.log

    # Print a line for each package in the transaction - after the transaction
    post_transaction:*:::/bin/sh -c echo\ 'post_transaction:\ ${{pkg.action}}\ ${{pkg.full_nevra}}\ repo\ ${{pkg.repo_id}}'\ >>\ {context.dnf.installroot}/actions.log

    # Print value of the temporary plugin variable "test_variable"
    post_transaction::::/bin/sh -c echo\ 'post_transaction:\ tmp.test_variable=${{tmp.test_variable}}'\ >>\ {context.dnf.installroot}/actions.log
    """
   When I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | setup-0:2.12.1-1.fc29.noarch          |
    And file "/actions.log" contents is
    """
    pre_base_setup: conf.defaultyes=0
    post_base_setup: var.test1=Value1 conf.defaultyes=1 tmp.test_variable=Value2
    post_base_setup: var.releasever=29
    pre_transaction: I setup-0:2.12.1-1.fc29.noarch repo dnf-ci-fedora
    post_transaction: I setup-0:2.12.1-1.fc29.noarch repo dnf-ci-fedora
    post_transaction: tmp.test_variable=Value_set_in_pre_transaction
    """


Scenario: Test substitutions and settings of repository options - the order of repositories in the list corresponds to their load order (random)
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    # Prints name of dnf-ci-fedora repository.
    repos_configured::::/usr/bin/sh -c echo\ 'pre_base_setup:\ "dnf-ci-fedora"\ repository\ name\ is:\ ${{conf.dnf-ci-fedora.name}}'\ >>\ {context.dnf.installroot}/actions.log

    # Prints a list of all configured repositories with their enable state.
    repos_configured::::/usr/bin/sh -c echo\ 'pre_base_setup:\ repositories:\ ${{conf.*.enabled}}'\ >>\ {context.dnf.installroot}/actions.log

    # Prints a list of configured repositories with "fedora" substring in the repository id with their enable state.
    repos_configured::::/usr/bin/sh -c echo\ 'pre_base_setup:\ repositories\ with\ "fedora"\ in\ id:\ ${{conf.*fedora*.enabled}}'\ >>\ {context.dnf.installroot}/actions.log

    # Prints a list of enabled repositories.
    repos_configured::::/usr/bin/sh -c echo\ 'pre_base_setup:\ enabled\ repositories:\ ${{conf.*.enabled=1}}'\ >>\ {context.dnf.installroot}/actions.log

    # Prints a list of enabled repositories with "fedora" substring in the repository id with their enable state.
    repos_configured::::/usr/bin/sh -c echo\ 'pre_base_setup:\ enabled\ repositories\ with\ "fedora"\ in\ id:\ ${{conf.*fedora*.enabled=1}}'\ >>\ {context.dnf.installroot}/actions.log

    # Prints a list of configured repositories with "thirdparty" substring in the repository name with their name. Escaped char in name ','.
    repos_configured::::/usr/bin/sh -c echo\ 'pre_base_setup:\ repositories\ with\ "thirdparty"\ in\ name:\ ${{conf.*.name=*thirdparty*}}'\ >>\ {context.dnf.installroot}/actions.log

    # Prints a list of configured repositories with "CI fedora" substring in the repository name with their name.
    repos_configured::::/usr/bin/sh -c echo\ 'pre_base_setup:\ repositories\ with\ "CI\ fedora"\ in\ name:\ ${{conf.*.name=*CI\ fedora*}}'\ >>\ {context.dnf.installroot}/actions.log

    # Disable all repositories
    repos_configured::::/usr/bin/sh -c echo\ 'conf.*.enabled=0'

    # Enable repository "dnf-ci-thirdparty"
    repos_configured::::/usr/bin/sh -c echo\ 'conf.dnf-ci-thirdparty.enabled=1'

    # Prints a list of enabled repositories.
    repos_configured::::/usr/bin/sh -c echo\ 'pre_base_setup:\ enabled\ repositories:\ ${{conf.*.enabled=1}}'\ >>\ {context.dnf.installroot}/actions.log
    """
   When I execute dnf with args "repo list --all"
   Then the exit code is 0
    And stdout is
      """
      repo id               repo name                          status
      dnf-ci-fedora         DNF CI fedora base repository    disabled
      dnf-ci-fedora-updates DNF CI fedora updates repository disabled
      dnf-ci-thirdparty     DNF CI thirdparty repo, test      enabled
      """
    And file "/actions.log" matches line by line
      """
      pre_base_setup: "dnf-ci-fedora" repository name is: dnf-ci-fedora.name=DNF CI fedora base repository
      ?pre_base_setup: repositories: dnf-ci-fedora.enabled=1,dnf-ci-fedora-updates.enabled=0,dnf-ci-thirdparty.enabled=0
      ?pre_base_setup: repositories: dnf-ci-fedora-updates.enabled=0,dnf-ci-fedora.enabled=1,dnf-ci-thirdparty.enabled=0
      ?pre_base_setup: repositories: dnf-ci-fedora.enabled=1,dnf-ci-thirdparty.enabled=0,dnf-ci-fedora-updates.enabled=0
      ?pre_base_setup: repositories: dnf-ci-fedora-updates.enabled=0,dnf-ci-thirdparty.enabled=0,dnf-ci-fedora.enabled=1
      ?pre_base_setup: repositories: dnf-ci-thirdparty.enabled=0,dnf-ci-fedora.enabled=1,dnf-ci-fedora-updates.enabled=0
      ?pre_base_setup: repositories: dnf-ci-thirdparty.enabled=0,dnf-ci-fedora-updates.enabled=0,dnf-ci-fedora.enabled=1
      ?pre_base_setup: repositories with "fedora" in id: dnf-ci-fedora.enabled=1,dnf-ci-fedora-updates.enabled=0
      ?pre_base_setup: repositories with "fedora" in id: dnf-ci-fedora-updates.enabled=0,dnf-ci-fedora.enabled=1
      pre_base_setup: enabled repositories: dnf-ci-fedora.enabled=1
      pre_base_setup: enabled repositories with "fedora" in id: dnf-ci-fedora.enabled=1
      pre_base_setup: repositories with "thirdparty" in name: dnf-ci-thirdparty.name=DNF CI thirdparty repo\\x2C test
      ?pre_base_setup: repositories with "CI fedora" in name: dnf-ci-fedora.name=DNF CI fedora base repository,dnf-ci-fedora-updates.name=DNF CI fedora updates repository
      ?pre_base_setup: repositories with "CI fedora" in name: dnf-ci-fedora-updates.name=DNF CI fedora updates repository,dnf-ci-fedora.name=DNF CI fedora base repository
      pre_base_setup: enabled repositories: dnf-ci-thirdparty.enabled=1
      """


Scenario Outline: I can filter on package or file: "<filter>"
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    pre_transaction:<filter>:::/bin/sh -c echo\ '${{pkg.action}}\ ${{pkg.full_nevra}}\ repo\ ${{pkg.repo_id}}'\ >>\ {context.dnf.installroot}/actions.log
    """
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
       | Action        | Package                                   |
       | install       | glibc-0:2.28-9.fc29.x86_64                |
       | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
       | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
       | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
       | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
       | install-dep   | basesystem-0:11-6.fc29.noarch             |
    And file "/actions.log" contents is
    """
    I glibc-0:2.28-9.fc29.x86_64 repo dnf-ci-fedora
    """

Examples:
    | filter            |
    | /etc/ld.so.conf   |
    | /etc/ld*conf      |
    | glibc             |
    | g*c               |


Scenario Outline: I can filter on transaction direction - inbound/outbound
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    post_transaction:*:<direction>::/bin/sh -c echo\ '${{pkg.action}}\ ${{pkg.name}}'\ >>\ {context.dnf.installroot}/actions.log
    """
    And I create file "/actions.log" with
    """
    """
   When I execute dnf with args "install setup"
   Then the exit code is 0
    And file "/actions.log" contents is
    """
    <output>
    """

Examples:
    | direction | output      |
    |           | I setup     |
    | in        | I setup     |
    | out       |             |


Scenario: Reason change is in transaction
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    pre_transaction:*:::/bin/sh -c echo\ '${{pkg.action}}\ ${{pkg.full_nevra}}\ repo\ ${{pkg.repo_id}}'\ >>\ {context.dnf.installroot}/actions.log
    """
    And I use repository "installonly"
    And I configure dnf with
        | key                          | value         |
        | installonlypkgs              | installonlyA  |
        | installonly_limit            | 2             |
   When I execute dnf with args "install installonlyA-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | installonlyA-1.0-1.x86_64       |
    And file "/actions.log" contents is
    """
    I installonlyA-0:1.0-1.x86_64 repo installonly
    """
   Given I delete file "/actions.log"
    And I execute dnf with args "install installonlyA-2.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | installonlyA-2.0-1.x86_64       |
    And file "/actions.log" contents is
    """
    I installonlyA-0:2.0-1.x86_64 repo installonly
    """
   Given I delete file "/actions.log"
    And I execute dnf with args "mark dependency installonlyA-2.0"
    And I execute dnf with args "install installonlyA-2.2"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | installonlyA-2.2-1.x86_64       |
        | remove        | installonlyA-1.0-1.x86_64       |
    And file "/actions.log" contains lines
    """
    E installonlyA-0:1.0-1.x86_64 repo @System
    \? installonlyA-0:2.0-1.x86_64 repo @System
    I installonlyA-0:2.2-1.x86_64 repo installonly
    """


Scenario: Testing the "raise_error" and a non-existent action process file
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    # Error logged.
    repos_configured::::/bin/non-existed ab c
    repos_configured:::raise_error=0:/bin/non-existed d ef

    # Exception thrown.
    repos_configured:::raise_error=1:/bin/non-existed xy z
    """
   When I execute dnf with args "install setup"
   Then the exit code is 1
    And stderr contains "Cannot execute action, command \"/bin/non-existed\" arguments \"xy z\""
    And file "/var/log/dnf5.log" contains lines
    """
    ERROR Actions plugin: .* Cannot execute action, command "/bin/non-existed" arguments "ab c"
    ERROR Actions plugin: .* Cannot execute action, command "/bin/non-existed" arguments "d ef"
    """


Scenario: Testing the "raise_error" option and exit codes
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    # Zero exit code. Success. No error was logged and no exception was thrown.
    repos_configured::::/bin/sh -c exit\ 0
    repos_configured:::raise_error=0:/bin/sh -c exit\ 0
    repos_configured:::raise_error=1:/bin/sh -c exit\ 0

    # Non-zero exit code. Error logged.
    repos_configured::::/bin/sh -c exit\ 1
    repos_configured:::raise_error=0:/bin/sh -c exit\ 2

    # Non-zero exit code. Exception thrown.
    repos_configured:::raise_error=1:/bin/sh -c exit\ 3
    """
   When I execute dnf with args "install setup"
   Then the exit code is 1
    And stderr contains "Exit code: 3"
    And file "/var/log/dnf5.log" does not contain lines
    """
    ERROR Actions plugin: .* Exit code: 0
    """
    And file "/var/log/dnf5.log" contains lines
    """
    ERROR Actions plugin: .* Exit code: 1
    ERROR Actions plugin: .* Exit code: 2
    """


Scenario: Testing the "raise_error" option and failed to process the output line in plain communication mode
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    # Error logged.
    repos_configured::::/bin/sh -c echo\ "conf.nonexist_option=1"
    repos_configured:::raise_error=0:/bin/sh -c echo\ "conf.nonexist_option=2"

    # Exception thrown.
    repos_configured:::raise_error=1:/bin/sh -c echo\ "conf.nonexist_option=3"
    """
   When I execute dnf with args "install setup"
   Then the exit code is 1
    And stderr contains "Cannot set option: Action output line: conf.nonexist_option=3"
    And file "/var/log/dnf5.log" contains lines
    """
    ERROR Actions plugin: .* Cannot set option: Action output line: conf.nonexist_option=1
    ERROR Actions plugin: .* Cannot set option: Action output line: conf.nonexist_option=2
    """


Scenario: Testing the "error" action message
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    # Error logged.
    repos_configured::::/bin/sh -c echo\ "error=Error\ in\ action\ process\ 1"
    repos_configured:::raise_error=0:/bin/sh -c echo\ "error=Error\ in\ action\ process\ 2"

    # Exception thrown.
    repos_configured:::raise_error=1:/bin/sh -c echo\ "error=Error\ in\ action\ process\ 3"
    """
   When I execute dnf with args "install setup"
   Then the exit code is 1
    And stderr contains "Action sent error message: Error in action process 3"
    And file "/var/log/dnf5.log" contains lines
    """
    ERROR Actions plugin: .* Action sent error message: Error in action process 1
    ERROR Actions plugin: .* Action sent error message: Error in action process 2
    """


Scenario: Testing the "stop" action request
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    # Exception thrown.
    repos_configured:::raise_error=1:/bin/sh -c echo\ "stop=I\ want\ to\ stop\ the\ task"
    """
   When I execute dnf with args "install setup"
   Then the exit code is 1
    And stderr contains "Action calls for stop: I want to stop the task"


Scenario: Testing the "stop" action request, must thrown exception even with "raise_error=0"
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    # Exception thrown.
    repos_configured:::raise_error=0:/bin/sh -c echo\ "stop=I\ want\ to\ stop\ the\ task"
    """
   When I execute dnf with args "install setup"
   Then the exit code is 1
    And stderr contains "Action calls for stop: I want to stop the task"


Scenario: Testing the "log" action request
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    # Error logged.
    repos_configured::::/bin/sh -c echo\ "log.TRACE=Test\ log\ message\ 1"
    repos_configured::::/bin/sh -c echo\ "log.DEBUG=Test\ log\ message\ 2"
    repos_configured::::/bin/sh -c echo\ "log.INFO=Test\ log\ message\ 3"
    repos_configured::::/bin/sh -c echo\ "log.NOTICE=Test\ log\ message\ 4"
    repos_configured::::/bin/sh -c echo\ "log.WARNING=Test\ log\ message\ 5"
    repos_configured::::/bin/sh -c echo\ "log.ERROR=Test\ log\ message\ 6"
    repos_configured::::/bin/sh -c echo\ "log.CRITICAL=Test\ log\ message\ 7"
    repos_configured:::raise_error=0:/bin/sh -c echo\ "log.BAD_LEVEL=Test\ log\ message\ 8"

    # Exception thrown.
    repos_configured:::raise_error=1:/bin/sh -c echo\ "log.BAD_LEVEL=Test\ log\ message\ 9"
    """
   When I execute dnf with args "install setup"
   Then the exit code is 1
    And stderr contains "Action sent the wrong log level: log.BAD_LEVEL=Test log message 9"
    And file "/var/log/dnf5.log" contains lines
    """
    TRACE Actions plugin: .* Test log message 1
    DEBUG Actions plugin: .* Test log message 2
    INFO Actions plugin: .* Test log message 3
    NOTICE Actions plugin: .* Test log message 4
    WARNING Actions plugin: .* Test log message 5
    ERROR Actions plugin: .* Test log message 6
    CRITICAL Actions plugin: .* Test log message 7
    ERROR Actions plugin: .* Action sent the wrong log level: log.BAD_LEVEL=Test log message 8
    """
