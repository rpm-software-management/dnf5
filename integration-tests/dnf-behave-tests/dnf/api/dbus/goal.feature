@dnf5daemon
@not.with_mode=dnf5
Feature: D-Bus api: goal

Scenario: Running a transaction before it is resolved raises an error
  Given I open a dnf5daemon session
   When I attempt to run the D-Bus transaction
   Then a D-Bus error "org.rpm.dnf.v0.rpm.Rpm.TransactionError" is raised with message "Transaction has to be resolved first."
