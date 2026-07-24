@not.with_dnf=4
Feature: transaction: install a package without dependencies


Scenario: Construct query and install labirinto package
Given I use repository "simple-base"
 When I execute python libdnf5 api script with setup
      """
      goal = libdnf5.base.Goal(base)
      goal.add_rpm_install("labirinto")
      execute_transaction(goal, "install a package without dependencies")
      """
 Then the exit code is 0
  And stdout is
      """
      labirinto-1.0-1.fc29.x86_64 : Install
      """
  And RPMDB Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |
