Feature: Store and replay transaction tests


Scenario: Replay a stored install transaction with relative path
Given I set working directory to "{context.dnf.tempdir}"
  And I use repository "transaction-sr"
 When I execute dnf with args "install top-a-1.0 --store ./install_trans"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                  |
      | install     | top-a-1:1.0-1.x86_64     |
      | install-dep | bottom-a2-0:1.0-1.x86_64 |
      | install-dep | bottom-a3-0:1.0-1.x86_64 |
      | install-dep | mid-a1-0:1.0-1.x86_64    |
      | install-weak | mid-a2-0:1.0-1.x86_64   |
  And RPMDB Transaction is empty
 When I execute dnf with args "replay ./install_trans"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | install     | top-a-1:1.0-1.x86_64     |
      | install-dep | bottom-a2-0:1.0-1.x86_64 |
      | install-dep | bottom-a3-0:1.0-1.x86_64 |
      | install-dep | mid-a1-0:1.0-1.x86_64    |
      | install-weak | mid-a2-0:1.0-1.x86_64   |
 When I execute dnf with args "rq --installed --qf '%{{full_nevra}} %{{reason}} %{{from_repo}}\n'"
 Then stdout is
      """
      bottom-a2-0:1.0-1.x86_64 Dependency @stored_transaction(transaction-sr)
      bottom-a3-0:1.0-1.x86_64 Dependency @stored_transaction(transaction-sr)
      mid-a1-0:1.0-1.x86_64 Dependency @stored_transaction(transaction-sr)
      mid-a2-0:1.0-1.x86_64 Weak Dependency @stored_transaction(transaction-sr)
      top-a-1:1.0-1.x86_64 User @stored_transaction(transaction-sr)
      """


Scenario: Replay a stored install transaction with absolute path
Given I use repository "transaction-sr"
 When I execute dnf with args "install top-a-1.0 --store {context.dnf.tempdir}/install_trans"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                  |
      | install     | top-a-1:1.0-1.x86_64     |
      | install-dep | bottom-a2-0:1.0-1.x86_64 |
      | install-dep | bottom-a3-0:1.0-1.x86_64 |
      | install-dep | mid-a1-0:1.0-1.x86_64    |
      | install-weak | mid-a2-0:1.0-1.x86_64   |
  And RPMDB Transaction is empty
 When I execute dnf with args "replay {context.dnf.tempdir}/install_trans"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | install     | top-a-1:1.0-1.x86_64     |
      | install-dep | bottom-a2-0:1.0-1.x86_64 |
      | install-dep | bottom-a3-0:1.0-1.x86_64 |
      | install-dep | mid-a1-0:1.0-1.x86_64    |
      | install-weak | mid-a2-0:1.0-1.x86_64   |
 When I execute dnf with args "rq --installed --qf '%{{full_nevra}} %{{reason}} %{{from_repo}}\n'"
 Then stdout is
      """
      bottom-a2-0:1.0-1.x86_64 Dependency @stored_transaction(transaction-sr)
      bottom-a3-0:1.0-1.x86_64 Dependency @stored_transaction(transaction-sr)
      mid-a1-0:1.0-1.x86_64 Dependency @stored_transaction(transaction-sr)
      mid-a2-0:1.0-1.x86_64 Weak Dependency @stored_transaction(transaction-sr)
      top-a-1:1.0-1.x86_64 User @stored_transaction(transaction-sr)
      """


Scenario: Replay a stored install transaction with no repos configured
Given I set working directory to "{context.dnf.tempdir}"
  And I use repository "transaction-sr"
 When I execute dnf with args "install top-a-1.0 --store ./install_trans"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                  |
      | install     | top-a-1:1.0-1.x86_64     |
      | install-dep | bottom-a2-0:1.0-1.x86_64 |
      | install-dep | bottom-a3-0:1.0-1.x86_64 |
      | install-dep | mid-a1-0:1.0-1.x86_64    |
      | install-weak | mid-a2-0:1.0-1.x86_64   |
  And RPMDB Transaction is empty
Given I drop repository "transaction-sr"
 When I execute dnf with args "replay ./install_trans"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | install     | top-a-1:1.0-1.x86_64     |
      | install-dep | bottom-a2-0:1.0-1.x86_64 |
      | install-dep | bottom-a3-0:1.0-1.x86_64 |
      | install-dep | mid-a1-0:1.0-1.x86_64    |
      | install-weak | mid-a2-0:1.0-1.x86_64   |
 When I execute dnf with args "rq --installed --qf '%{{full_nevra}} %{{reason}} %{{from_repo}}\n'"
 Then stdout is
      """
      bottom-a2-0:1.0-1.x86_64 Dependency @stored_transaction(transaction-sr)
      bottom-a3-0:1.0-1.x86_64 Dependency @stored_transaction(transaction-sr)
      mid-a1-0:1.0-1.x86_64 Dependency @stored_transaction(transaction-sr)
      mid-a2-0:1.0-1.x86_64 Weak Dependency @stored_transaction(transaction-sr)
      top-a-1:1.0-1.x86_64 User @stored_transaction(transaction-sr)
      """
