Feature: Transaction history undo

# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
@bz1700529
Scenario: Undo module install with dependent userinstalled package
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-modular"
   # install module that contains postgresql-server
    And I successfully execute dnf with args "module enable postgresql/server"
    And I successfully execute dnf with args "install postgresql-server-0:9.6.8-1.module_1710+b535a823.x86_64"
   # install package, that requires postgresql-server
    And I successfully execute dnf with args "install postgresql-test"
   # try to undo module install transaction
   When I execute dnf with args "history undo last-1"
   # the transaction is not supposed to reinstall required packages, but to fail
   Then the exit code is 1
    And stdout is empty
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Extra package 'postgresql-test-9.6.8-1.module_1710+b535a823.x86_64' (with action 'Remove') which is not present in the stored transaction was pulled into the transaction.

        You can try to add to command line:
          --ignore-extras to allow extra packages in the transaction
        """
