Feature: transaction: dry-run a transaction


Background:
Given I use repository "simple-base"


@bz2109660
Scenario: Test labirinto install transaction when it should succeed
 When I execute python libdnf5 api script with setup
      """
      goal = libdnf5.base.Goal(base)
      goal.add_rpm_install("labirinto")
      assert test_transaction(goal) == libdnf5.base.Transaction.TransactionRunResult_SUCCESS
      """
 Then the exit code is 0
  And stderr is empty


@bz2109660
Scenario: Test labirinto install transaction when it should fail
 When I execute python libdnf5 api script with setup
      """
      goal = libdnf5.base.Goal(base)
      goal.add_rpm_install("labirinto")
      execute_transaction(goal, "install a package")

      # We have already run the transaction, so test_transaction should return a failing status
      assert test_transaction(goal) == libdnf5.base.Transaction.TransactionRunResult_ERROR_RPM_RUN
      """
 Then the exit code is 0
  And stderr is empty


Scenario: Test transaction that removes not installed labirinto package
 When I execute python libdnf5 api script with setup
      """
      goal = libdnf5.base.Goal(base)
      goal.add_rpm_remove("labirinto")
      # it is supposed to be OK
      assert test_transaction(goal) == libdnf5.base.Transaction.TransactionRunResult_SUCCESS
      """
 Then the exit code is 0
  And stderr is empty


Scenario: Test transaction installing labirinto and a non-existent package
 When I execute python libdnf5 api script with setup
      """
      goal = libdnf5.base.Goal(base)
      goal.add_rpm_install("non-existent-install")
      goal.add_rpm_install("labirinto")
      assert test_transaction(goal) == libdnf5.base.Transaction.TransactionRunResult_ERROR_RESOLVE
      """
 Then the exit code is 0
  And stderr is empty


Scenario: Test transaction containing improper argument
 When I execute python libdnf5 api script with setup
      """
      goal = libdnf5.base.Goal(base)
      goal.add_rpm_install(99)
      # traceback is expected
      test_transaction(goal)
      """
 Then the exit code is 1
  And stderr contains "TypeError: Wrong number or type of arguments for overloaded function 'Goal_add_rpm_install'."
