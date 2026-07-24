Feature: Handling of errors on the history database

Background:
Given I use repository "dnf-ci-fedora"


Scenario: history list on a broken history database
Given I create file "/usr/lib/sysimage/libdnf5/transaction_history.sqlite" with
      """
      GARBAGE
      """
 When I execute dnf with args "history list"
 Then the exit code is 1
  And stderr is
      """
      SQL statement execution failed: "PRAGMA locking_mode = NORMAL; PRAGMA journal_mode = WAL; PRAGMA foreign_keys = ON;": (26) - file is not a database
      """


Scenario: install with a broken history database
Given I use repository "dnf-ci-fedora"
  And I create file "/usr/lib/sysimage/libdnf5/transaction_history.sqlite" with
      """
      GARBAGE
      """
 When I execute dnf with args "install filesystem"
 Then the exit code is 1
  And stderr contains "SQL statement execution failed: "PRAGMA locking_mode = NORMAL; PRAGMA journal_mode = WAL; PRAGMA foreign_keys = ON;": \(26\) - file is not a database"


@bz1634385
@no_installroot
Scenario: history database not present under a regular user, who has write permission
Given I successfully execute "chmod o+rwx {context.dnf.tempdir}"
 When I execute dnf with args "--setopt=system_state_dir={context.dnf.tempdir} history list" as an unprivileged user
 Then the exit code is 0
  And stderr is empty
  And stdout is empty
  And file "{context.dnf.tempdir}/transaction_history.sqlite" exists


@bz1634385
@no_installroot
Scenario: history database not present under a regular user
 When I execute dnf with args "--setopt=system_state_dir={context.dnf.tempdir} history list" as an unprivileged user
 Then the exit code is 1
  And stderr is
      """
      Failed to open database "{context.dnf.tempdir}/transaction_history.sqlite": (14) - unable to open database file
      """


@bz1761976
@no_installroot
Scenario: read permission error on the history database
Given I successfully execute dnf with args "--setopt=system_state_dir={context.dnf.tempdir} install abcde"
  And I successfully execute "chmod o-r {context.dnf.tempdir}/transaction_history.sqlite"
 When I execute dnf with args "--setopt=system_state_dir={context.dnf.tempdir} history list" as an unprivileged user
 Then the exit code is 1
  And stderr is
      """
      Failed to open database "{context.dnf.tempdir}/transaction_history.sqlite": (14) - unable to open database file
      """


@bz1761976
@no_installroot
Scenario: read permission error on the history database directory
Given I successfully execute dnf with args "--setopt=system_state_dir={context.dnf.tempdir} install abcde"
  # executable permission on directory means its contents can't be read
  And I successfully execute "chmod o-x {context.dnf.tempdir}"
 When I execute dnf with args "--setopt=system_state_dir={context.dnf.tempdir} history list" as an unprivileged user
 Then the exit code is 1
  And stderr is
      """
      Failed to open database "{context.dnf.tempdir}/transaction_history.sqlite": (14) - unable to open database file
      """
