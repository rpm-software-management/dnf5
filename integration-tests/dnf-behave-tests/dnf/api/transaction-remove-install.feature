@not.with_dnf=4
Feature: transaction: remove a package with dependency, then install the dependency again


Scenario: Construct query, remove vagare, then install labirinto
Given I use repository "simple-base"
Given I successfully execute dnf with args "install vagare"
# vagare depends on labirinto
 When I execute python libdnf5 api script with setup
      """
      goal = libdnf5.base.Goal(base)
      goal.add_rpm_remove("vagare")
      goal.add_rpm_install("labirinto")
      execute_transaction(goal, "remove and install")
      """
 Then the exit code is 0
  And stdout is
      """
      vagare-1.0-1.fc29.x86_64 : Remove
      """
  And RPMDB Transaction is following
      | Action        | Package                       |
      | remove        | vagare-0:1.0-1.fc29.x86_64    |
