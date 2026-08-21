Feature: Always use the latest packages when installing dependecies


Scenario: prefer installing latests dependencies rather than smaller transaction
  Given I use repository "install-latest-deps"
    And I successfully execute dnf with args "install krb5-libs-1.0"
   When I execute dnf with args "install ipa-client"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | ipa-client-0:1.0-1.fc29.x86_64  |
        | install-dep   | krb5-pkinit-0:2.0-1.fc29.x86_64 |
        | upgrade       | krb5-libs-0:2.0-1.fc29.x86_64   |


Scenario: if latests dependencies are not possible to install fall back to lower versions without errors
  Given I use repository "install-latest-deps"
    And I successfully execute dnf with args "install krb5-libs-1.0"
    When I execute dnf with args "install ipa-client -x krb5-libs-2.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | ipa-client-0:1.0-1.fc29.x86_64  |
        | install-dep   | krb5-pkinit-0:1.0-1.fc29.x86_64 |
