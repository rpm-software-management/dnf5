Feature: Transaction replay tests

Background:
Given I set working directory to "{context.dnf.tempdir}"
Given I use repository "transaction-sr"
  And I successfully execute dnf with args "install top-a-1.0"


Scenario: Replay an install transaction
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "top-d-1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                    |
      | install-dep | bottom-a1-0:2.0-1.noarch   |
      | install     | top-d-0:1.0-1.x86_64       |
  And dnf5 transaction items for transaction "last" are
      | action      | package                           | reason        | repository     |
      | Install     | bottom-a1-0:2.0-1.noarch          | Dependency    | transaction-sr |
      | Install     | top-d-0:1.0-1.x86_64              | User          | transaction-sr |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | User            |
      | top-d-1.0-1.x86_64     | User            |


Scenario: Replay an install transaction from a non-existent repository
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "User",
                  "repo_id": "nonexistent"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | install     | bottom-a1-2.0-1.noarch   |
  And dnf5 transaction items for transaction "last" are
      | action        | package                           | reason        | repository     |
      | Install       | bottom-a1-0:2.0-1.noarch          | User          | transaction-sr |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | User            |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | User            |


Scenario: Replay an upgrade transaction
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Upgrade",
                  "nevra": "top-a-1:2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | install-dep | bottom-a1-2.0-1.noarch   |
      | upgrade     | top-a-1:2.0-1.x86_64     |
  And dnf5 transaction items for transaction "last" are
      | action        | package                  | reason     | repository     |
      | Install       | bottom-a1-0:2.0-1.noarch | Dependency | transaction-sr |
      | Upgrade       | top-a-1:2.0-1.x86_64     | User       | transaction-sr |
      | Replaced      | top-a-1:1.0-1.x86_64     | User       | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:2.0-1.x86_64   | User            |


Scenario: Replay a reinstall transaction
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Reinstall",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | reinstall   | top-a-1:1.0-1.x86_64     |
  And dnf5 transaction items for transaction "last" are
      | action    | package                   | reason | repository     |
      | Reinstall | top-a-1:1.0-1.x86_64      | User   | transaction-sr |
      | Replaced  | top-a-1:1.0-1.x86_64      | User   | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | User            |



Scenario: Replay a downgrade transaction
Given I successfully execute dnf with args "upgrade top-a"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Downgrade",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-a-1:2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | downgrade   | top-a-1:1.0-1.x86_64     |
  And dnf5 transaction items for transaction "last" are
      | action    | package                   | reason | repository     |
      | Downgrade | top-a-1:1.0-1.x86_64      | User   | transaction-sr |
      | Replaced  | top-a-1:2.0-1.x86_64      | User   | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | User            |


Scenario: Replay a remove transaction
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Remove",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | remove      | top-a-1:1.0-1.x86_64     |
  And dnf5 transaction items for transaction "last" are
      | action    | package                   | reason | repository     |
      | Remove    | top-a-1:1.0-1.x86_64      | User   | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |


Scenario: Replay a reason change transaction
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Reason Change",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action          | Package                  |
      | changing-reason | top-a-1:1.0-1.x86_64     |
  And dnf5 transaction items for transaction "last" are
      | action        | package                   | reason     | repository     |
      | Reason Change | top-a-1:1.0-1.x86_64      | Dependency | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | Dependency      |


Scenario: Replay a reason change transaction on a not-installed package
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Reason Change",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Argument 'top-b-1.0-1.x86_64' matches only excluded packages.
      """


Scenario: Replay a reason change transaction on a package being installed
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Reason Change",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Argument 'top-b-1.0-1.x86_64' matches only excluded packages.
      """


Scenario: Replay a reason change transaction on a package being removed
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Reason Change",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Remove",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And dnf5 transaction items for transaction "last" are
      | action        | package                   | reason     | repository     |
      | Remove        | top-a-1:1.0-1.x86_64      | User       | @System        |
      | Reason Change | top-a-1:1.0-1.x86_64      | Dependency | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |


Scenario: Replay installing a group
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Install",
                  "id": "test-group",
                  "package_types": "conditional, default, mandatory",
                  "reason": "User",
              }
          ],
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "Group",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Upgrade",
                  "nevra": "top-a-1:2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                  |
      | upgrade       | top-a-1:2.0-1.x86_64     |
      | install-group | top-b-1.0-1.x86_64       |
      | install-dep   | bottom-a1-2.0-1.noarch   |
      | group-install | Test Group               |
  And dnf5 transaction items for transaction "last" are
      | action        | package                   | reason     | repository     |
      | Install       | bottom-a1-0:2.0-1.noarch  | Dependency | transaction-sr |
      | Install       | top-b-0:1.0-1.x86_64      | Group      | transaction-sr |
      | Upgrade       | top-a-1:2.0-1.x86_64      | User       | transaction-sr |
      | Replaced      | top-a-1:1.0-1.x86_64      | User       | @System        |
      | Install       | test-group                | User       | transaction-sr |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:2.0-1.x86_64   | User            |
      | top-b-1.0-1.x86_64     | Group           |
  And group state is
      | id         | package_types                   | packages | userinstalled |
      | test-group | conditional, default, mandatory | top-b    | True          |


Scenario: Replay installing a group without the `default` package type
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Install",
                  "id": "test-group",
                  "package_types": "conditional, mandatory",
                  "reason": "User",
              }
          ],
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Upgrade",
                  "nevra": "top-a-1:2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                  |
      | upgrade       | top-a-1:2.0-1.x86_64     |
      | install-dep   | bottom-a1-2.0-1.noarch   |
      | group-install | Test Group               |
  And dnf5 transaction items for transaction "last" are
      | action        | package                   | reason     | repository     |
      | Install       | bottom-a1-0:2.0-1.noarch  | Dependency | transaction-sr |
      | Upgrade       | top-a-1:2.0-1.x86_64      | User       | transaction-sr |
      | Replaced      | top-a-1:1.0-1.x86_64      | User       | @System        |
      | Install       | test-group                | User       | transaction-sr |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:2.0-1.x86_64   | User            |
  And group state is
      | id         | package_types          | packages | userinstalled |
      | test-group | conditional, mandatory |          | True          |


Scenario: Replay removing a group
Given I successfully execute dnf with args "install @test-group"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Remove",
                  "id": "test-group",
                  "package_types": "conditional, default, mandatory",
                  "reason": "User",
              }
          ],
          "rpms": [
              {
                  "action": "Remove",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "Clean",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                  |
      | remove-unused | top-b-1.0-1.x86_64       |
      | group-remove  | Test Group               |
  And dnf5 transaction items for transaction "last" are
      | action        | package                   | reason     | repository     |
      | Remove        | top-b-0:1.0-1.x86_64      | Clean      | @System        |
      | Remove        | test-group                | User       | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:2.0-1.x86_64   | User            |
  And group state is
      | id | package_types | packages | userinstalled |


Scenario: Replay upgrading a group
Given I successfully execute dnf with args "install @test-group"
  And I successfully execute dnf with args "install top-a-1.0"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Upgrade",
                  "id": "test-group",
                  "package_types": "conditional, default, mandatory",
                  "reason": "User",
              }
          ],
          "rpms": [
              {
                  "action": "Upgrade",
                  "nevra": "top-a-1:2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                  |
      | upgrade       | top-a-1:2.0-1.x86_64     |
      | group-upgrade | Test Group               |
  And dnf5 transaction items for transaction "last" are
      | action        | package                   | reason     | repository     |
      | Upgrade       | top-a-1:2.0-1.x86_64      | User       | transaction-sr |
      | Replaced      | top-a-1:1.0-1.x86_64      | User       | @System        |
      | Upgrade       | test-group                | User       | transaction-sr |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:2.0-1.x86_64   | User            |
      | top-b-1.0-1.x86_64     | Group           |
  And group state is
      | id         | package_types                   | packages | userinstalled |
      | test-group | conditional, default, mandatory | top-b    | True          |


Scenario: Replay installing an environment
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "action": "Install",
                  "id": "test-env",
                  "package_types": "conditional, default, mandatory"
              }
          ],
          "groups": [
              {
                  "action": "Install",
                  "id": "test-env-group",
                  "package_types": "conditional, default, mandatory",
                  "reason": "Dependency",
              }
          ],
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "top-c-2.0-1.x86_64",
                  "reason": "Group",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Upgrade",
                  "nevra": "mid-a2-2.0-1.x86_64",
                  "reason": "Weak Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "mid-a2-1.0-1.x86_64",
                  "reason": "Weak Dependency",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                  |
      | upgrade       | mid-a2-2.0-1.x86_64      |
      | install-group | top-c-2.0-1.x86_64       |
      | env-install   | Test Environment         |
      | group-install | Test Env Group           |
  And dnf5 transaction items for transaction "last" are
      | action        | package               | reason          | repository     |
      | Install       | top-c-0:2.0-1.x86_64  | Group           | transaction-sr |
      | Upgrade       | mid-a2-0:2.0-1.x86_64 | Weak Dependency | transaction-sr |
      | Replaced      | mid-a2-0:1.0-1.x86_64 | Weak Dependency | @System        |
      | Install       | test-env-group        | Dependency      | transaction-sr |
      | Install       | test-env              | User            | transaction-sr |
  And package reasons are
      | Package                | Reason          |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-2.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | User            |
      | top-c-2.0-1.x86_64     | Group           |
  And environment state is
      | id       | groups         |
      | test-env | test-env-group |


Scenario: Replay removing an environment group
Given I successfully execute dnf with args "install @test-env"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "action": "Remove",
                  "id": "test-env",
                  "package_types": "conditional, default, mandatory"
              }
          ],
          "groups": [
              {
                  "action": "Remove",
                  "id": "test-env-group",
                  "package_types": "conditional, default, mandatory",
                  "reason": "Dependency",
              }
          ],
          "rpms": [
              {
                  "action": "Remove",
                  "nevra": "bottom-a3-1.0-1.x86_64",
                  "reason": "Clean",
                  "repo_id": "@System"
              },
              {
                  "action": "Remove",
                  "nevra": "mid-a2-2.0-1.x86_64",
                  "reason": "Clean",
                  "repo_id": "@System"
              },
              {
                  "action": "Remove",
                  "nevra": "top-c-2.0-1.x86_64",
                  "reason": "Group",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                  |
      | remove        | top-c-2.0-1.x86_64       |
      | remove-unused | bottom-a3-1.0-1.x86_64   |
      | remove-unused | mid-a2-2.0-1.x86_64      |
      | group-remove  | Test Env Group           |
      | env-remove    | Test Environment         |
  And dnf5 transaction items for transaction "last" are
      | action        | package                  | reason          | repository     |
      | Remove        | bottom-a3-0:1.0-1.x86_64 | Clean           | @System        |
      | Remove        | mid-a2-0:2.0-1.x86_64    | Clean           | @System        |
      | Remove        | top-c-0:2.0-1.x86_64     | Group           | @System        |
      | Remove        | test-env-group           | Dependency      | @System        |
      | Remove        | test-env                 | User            | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | top-a-1:1.0-1.x86_64   | User            |
  And environment state is
      | id       | groups |


Scenario: Replay upgrading an environment group
Given I successfully execute dnf with args "install @test-env"
  And I successfully execute dnf with args "install top-c-1.0"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "action": "Upgrade",
                  "id": "test-env",
                  "package_types": "conditional, default, mandatory"
              }
          ],
          "groups": [
              {
                  "action": "Upgrade",
                  "id": "test-env-group",
                  "package_types": "conditional, default, mandatory",
                  "reason": "Dependency",
              }
          ],
          "rpms": [
              {
                  "action": "Upgrade",
                  "nevra": "mid-a2-2.0-1.x86_64",
                  "reason": "Weak Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "mid-a2-1.0-1.x86_64",
                  "reason": "Weak Dependency",
                  "repo_id": "@System"
              },
              {
                  "action": "Upgrade",
                  "nevra": "top-c-2.0-1.x86_64",
                  "reason": "Group",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-c-1.0-1.x86_64",
                  "reason": "Group",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                  |
      | upgrade       | top-c-2.0-1.x86_64       |
      | upgrade       | mid-a2-2.0-1.x86_64      |
      | group-upgrade | Test Env Group           |
      | env-upgrade   | Test Environment         |
  And dnf5 transaction items for transaction "last" are
      | action        | package                  | reason          | repository     |
      | Upgrade       | mid-a2-0:2.0-1.x86_64    | Weak Dependency | transaction-sr |
      | Upgrade       | top-c-0:2.0-1.x86_64     | Group           | transaction-sr |
      | Replaced      | mid-a2-0:1.0-1.x86_64    | Weak Dependency | @System        |
      | Replaced      | top-c-0:1.0-1.x86_64     | Group           | @System        |
      | Upgrade       | test-env-group           | Dependency      | transaction-sr |
      | Upgrade       | test-env                 | User            | transaction-sr |
  And package reasons are
      | Package                | Reason          |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-2.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | User            |
      | top-c-2.0-1.x86_64     | Group           |
  And environment state is
      | id       | groups         |
      | test-env | test-env-group |


Scenario: Replay a transaction installing multiple installonly packages
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "installonly-1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "installonly-2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                    |
      | install     | installonly-0:1.0-1.x86_64 |
      | install     | installonly-0:2.0-1.x86_64 |
  And dnf5 transaction items for transaction "last" are
      | action        | package                    | reason  | repository     |
      | Install       | installonly-0:1.0-1.x86_64 | User    | transaction-sr |
      | Install       | installonly-0:2.0-1.x86_64 | User    | transaction-sr |
  And package reasons are
      | Package                  | Reason          |
      | bottom-a2-1.0-1.x86_64   | Dependency      |
      | bottom-a3-1.0-1.x86_64   | Dependency      |
      | installonly-1.0-1.x86_64 | User            |
      | installonly-2.0-1.x86_64 | User            |
      | mid-a1-1.0-1.x86_64      | Dependency      |
      | mid-a2-1.0-1.x86_64      | Weak Dependency |
      | top-a-1:1.0-1.x86_64     | User            |


Scenario: Replay a transaction removing multiple installonly packages
Given I successfully execute dnf with args "install installonly-1.0 installonly-2.0"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Remove",
                  "nevra": "installonly-2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              },
              {
                  "action": "Remove",
                  "nevra": "installonly-1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | remove      | installonly-1.0-1.x86_64 |
      | remove      | installonly-2.0-1.x86_64 |
  And dnf5 transaction items for transaction "last" are
      | action        | package                    | reason  | repository |
      | Remove        | installonly-0:2.0-1.x86_64 | User    | @System    |
      | Remove        | installonly-0:1.0-1.x86_64 | User    | @System    |
  And package reasons are
      | Package                | Reason          |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | User            |


Scenario: Replay a transaction installing and removing an installonly package
Given I successfully execute dnf with args "install installonly-1.0"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "installonly-2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Remove",
                  "nevra": "installonly-1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | install     | installonly-2.0-1.x86_64 |
      | remove      | installonly-1.0-1.x86_64 |
  And dnf5 transaction items for transaction "last" are
      | action        | package                    | reason  | repository     |
      | Install       | installonly-0:2.0-1.x86_64 | User    | transaction-sr |
      | Remove        | installonly-0:1.0-1.x86_64 | User    | @System        |
  And package reasons are
      | Package                  | Reason          |
      | bottom-a2-1.0-1.x86_64   | Dependency      |
      | bottom-a3-1.0-1.x86_64   | Dependency      |
      | installonly-2.0-1.x86_64 | User            |
      | mid-a1-1.0-1.x86_64      | Dependency      |
      | mid-a2-1.0-1.x86_64      | Weak Dependency |
      | top-a-1:1.0-1.x86_64     | User            |


Scenario: Replay a transaction obsoleting a package
Given I successfully execute dnf with args "install obsoleted-a-1.0"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "obsoleting-x-2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "obsoleted-a-1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                   |
      | install     | obsoleting-x-2.0-1.x86_64 |
      | obsoleted   | obsoleted-a-1.0-1.x86_64  |
  And dnf5 transaction items for transaction "last" are
      | action        | package                     | reason  | repository     |
      | Install       | obsoleting-x-0:2.0-1.x86_64 | User    | transaction-sr |
      | Replaced      | obsoleted-a-0:1.0-1.x86_64  | User    | @System        |
  And package reasons are
      | Package                   | Reason          |
      | bottom-a2-1.0-1.x86_64    | Dependency      |
      | bottom-a3-1.0-1.x86_64    | Dependency      |
      | mid-a1-1.0-1.x86_64       | Dependency      |
      | mid-a2-1.0-1.x86_64       | Weak Dependency |
      | obsoleting-x-2.0-1.x86_64 | User            |
      | top-a-1:1.0-1.x86_64      | User            |


Scenario: Replay a transaction obsoleting multiple packages
Given I successfully execute dnf with args "install obsoleted-a-1.0 obsoleted-b-1.0"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "obsoleting-x-2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "obsoleted-a-1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              },
              {
                  "action": "Replaced",
                  "nevra": "obsoleted-b-1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              },
              {
                  "action": "Install",
                  "nevra": "obsoleting-y-2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                   |
      | install     | obsoleting-x-2.0-1.x86_64 |
      | install     | obsoleting-y-2.0-1.x86_64 |
      | obsoleted   | obsoleted-a-1.0-1.x86_64  |
      | obsoleted   | obsoleted-b-1.0-1.x86_64  |
  And dnf5 transaction items for transaction "last" are
      | action        | package                     | reason  | repository     |
      | Install       | obsoleting-x-0:2.0-1.x86_64 | User    | transaction-sr |
      | Install       | obsoleting-y-0:2.0-1.x86_64 | User    | transaction-sr |
      | Replaced      | obsoleted-a-0:1.0-1.x86_64  | User    | @System        |
      | Replaced      | obsoleted-b-0:1.0-1.x86_64  | User    | @System        |
  And package reasons are
      | Package                   | Reason          |
      | bottom-a2-1.0-1.x86_64    | Dependency      |
      | bottom-a3-1.0-1.x86_64    | Dependency      |
      | mid-a1-1.0-1.x86_64       | Dependency      |
      | mid-a2-1.0-1.x86_64       | Weak Dependency |
      | obsoleting-x-2.0-1.x86_64 | User            |
      | obsoleting-y-2.0-1.x86_64 | User            |
      | top-a-1:1.0-1.x86_64      | User            |


Scenario: Replay an upgrade transaction where a package that is being upgraded has a different reason
Given I successfully execute dnf with args "install bottom-a1-1.0"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Upgrade",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "bottom-a1-1.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "@System"
              },
              {
                  "action": "Upgrade",
                  "nevra": "top-a-1:2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                  |
      | upgrade     | bottom-a1-2.0-1.noarch   |
      | upgrade     | top-a-1:2.0-1.x86_64     |
  And dnf5 transaction items for transaction "last" are
      | action        | package                     | reason     | repository     |
      | Upgrade       | bottom-a1-0:2.0-1.noarch    | User       | transaction-sr |
      | Upgrade       | top-a-1:2.0-1.x86_64        | User       | transaction-sr |
      | Replaced      | bottom-a1-0:1.0-1.noarch    | User       | @System        |
      | Replaced      | top-a-1:1.0-1.x86_64        | User       | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | User            |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:2.0-1.x86_64   | User            |


Scenario: Replay a transaction with an arch change
Given I successfully execute dnf with args "install archchange-1.0"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Upgrade",
                  "nevra": "archchange-2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "archchange-1.0-1.noarch",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And DNF Transaction is following
      | Action      | Package                   |
      | upgrade     | archchange-2.0-1.x86_64   |
  And RPMDB Transaction is following
      | Action      | Package                   |
      | remove      | archchange-1.0-1.noarch   |
      | install     | archchange-2.0-1.x86_64   |
  And dnf5 transaction items for transaction "last" are
      | action        | package                     | reason | repository     |
      | Upgrade       | archchange-0:2.0-1.x86_64   | User   | transaction-sr |
      | Replaced      | archchange-0:1.0-1.noarch   | User   | @System        |
  And package reasons are
      | Package                 | Reason          |
      | archchange-2.0-1.x86_64 | User            |
      | bottom-a2-1.0-1.x86_64  | Dependency      |
      | bottom-a3-1.0-1.x86_64  | Dependency      |
      | mid-a1-1.0-1.x86_64     | Dependency      |
      | mid-a2-1.0-1.x86_64     | Weak Dependency |
      | top-a-1:1.0-1.x86_64    | User            |


# This should fail or there should be at least a warning.
# Reported as https://github.com/rpm-software-management/dnf5/issues/1573
@xfail
Scenario: Replay a transaction with multiple actions per NEVRA
Given I successfully execute dnf with args "install @test-group supertop-b"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Remove",
                  "id": "test-group",
                  "package_types": "conditional, default, mandatory",
                  "reason": "User",
              }
          ],
          "rpms": [
              {
                  "action": "Reason Change",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Reinstall",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "Group",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "Group",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
  """
  Something like: Cannot perform multiple actions for: 'top-b-1.0-1.x86_64', only one action per nevra is possible.
  """


Scenario: ignore-installed: Replay an upgrade transaction where a package that is being installed is already on the system in a lower version
Given I successfully execute dnf with args "install bottom-a1-1.0"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Upgrade",
                  "nevra": "top-a-1:2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction --ignore-installed"
 Then the exit code is 0
  And stderr contains lines
      """
      Cannot perform Install action because 'bottom-a1-2.0-1.noarch' is installed in a different version: 'bottom-a1-0:1.0-1.noarch'.
      """
  And Transaction is following
      | Action      | Package                  |
      | upgrade     | bottom-a1-0:2.0-1.noarch |
      | upgrade     | top-a-1:2.0-1.x86_64     |
  And dnf5 transaction items for transaction "last" are
      | action        | package                  | reason     | repository     |
      | Upgrade       | bottom-a1-0:2.0-1.noarch | User       | transaction-sr |
      | Upgrade       | top-a-1:2.0-1.x86_64     | User       | transaction-sr |
      | Replaced      | bottom-a1-0:1.0-1.noarch | User       | @System        |
      | Replaced      | top-a-1:1.0-1.x86_64     | User       | @System        |
  And package reasons are
      | Package                  | Reason          |
      | bottom-a1-2.0-1.noarch   | User            |
      | bottom-a2-1.0-1.x86_64   | Dependency      |
      | bottom-a3-1.0-1.x86_64   | Dependency      |
      | mid-a1-1.0-1.x86_64      | Dependency      |
      | mid-a2-1.0-1.x86_64      | Weak Dependency |
      | top-a-1:2.0-1.x86_64     | User            |


Scenario: ignore-installed: Replaying an already installed transaction results in noop
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a2-1.0-1.x86_64",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "bottom-a3-1.0-1.x86_64",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "mid-a1-1.0-1.x86_64",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "mid-a2-1.0-1.x86_64",
                  "reason": "Weak Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction --ignore-installed --setopt=skip_unavailable=0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      Package "bottom-a2-0:1.0-1.x86_64" is already installed.
      Package "bottom-a3-0:1.0-1.x86_64" is already installed.
      Package "mid-a1-0:1.0-1.x86_64" is already installed.
      Package "mid-a2-0:1.0-1.x86_64" is already installed.
      Package "top-a-1:1.0-1.x86_64" is already installed.
     """
  And Transaction is empty


Scenario: ignore-installed: Replay an upgrade transaction where a package that is being upgraded is not installed on the system
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Upgrade",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "bottom-a1-0:1.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "@System"
              },
              {
                  "action": "Upgrade",
                  "nevra": "top-a-1:2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction --ignore-installed"
 Then the exit code is 0
 And stderr contains lines
     """
     Cannot perform Remove action for Package 'bottom-a1-0:1.0-1.noarch' because it is not installed.
     """
  And Transaction is following
      | Action      | Package                  |
      | install-dep | bottom-a1-2.0-1.noarch   |
      | upgrade     | top-a-1:2.0-1.x86_64     |
  And dnf5 transaction items for transaction "last" are
      | action        | package                  | reason     | repository     |
      | Install       | bottom-a1-0:2.0-1.noarch | Dependency | transaction-sr |
      | Upgrade       | top-a-1:2.0-1.x86_64     | User       | transaction-sr |
      | Replaced      | top-a-1:1.0-1.x86_64     | User       | @System        |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:2.0-1.x86_64   | User            |


Scenario: ignore-installed: Replay a remove transaction where a package that is being removed is not installed on the system
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Remove",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction --ignore-installed --skip-unavailable"
 Then the exit code is 0
 And stderr is
     """
     <REPOSYNC>
     Cannot perform Remove action for Package 'bottom-a1-2.0-1.noarch' because it is not installed.
     """
  And Transaction is empty


Scenario: skip-unavailable: Replay a transaction installing a nonexistent package
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "does-not-exist-1.0-1.noarch",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction --skip-unavailable"
 Then the exit code is 0
  And stderr contains lines
      """
      Cannot perform Install action, no match for: does-not-exist-1.0-1.noarch.
      """
  And Transaction is following
      | Action      | Package                  |
      | install-dep | bottom-a1-2.0-1.noarch   |
  And dnf5 transaction items for transaction "last" are
      | action        | package                  | reason     | repository     |
      | Install       | bottom-a1-0:2.0-1.noarch | Dependency | transaction-sr |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | User            |


Scenario: skip-unavailable: Replay a transaction reinstalling a non-available package
Given I successfully execute dnf with args "install bottom-a1-2.0"
  And I drop repository "transaction-sr"
  And I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Reinstall",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "@System"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction --skip-unavailable"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      Packages for argument 'bottom-a1-2.0-1.noarch' installed, but not available.
      """
  And Transaction is empty


Scenario: skip-broken: Replay a transaction with a broken dependency
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a1-2.0-1.noarch",
                  "reason": "Dependency",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Install",
                  "nevra": "broken-dep-1.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction --skip-broken"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                   |
      | install-dep | bottom-a1-2.0-1.noarch    |
      | broken      | broken-dep-0:1.0-1.x86_64 |
  And dnf5 transaction items for transaction "last" are
      | action        | package                  | reason     | repository     |
      | Install       | bottom-a1-0:2.0-1.noarch | Dependency | transaction-sr |
  And package reasons are
      | Package                | Reason          |
      | bottom-a1-2.0-1.noarch | Dependency      |
      | bottom-a2-1.0-1.x86_64 | Dependency      |
      | bottom-a3-1.0-1.x86_64 | Dependency      |
      | mid-a1-1.0-1.x86_64    | Dependency      |
      | mid-a2-1.0-1.x86_64    | Weak Dependency |
      | top-a-1:1.0-1.x86_64   | User            |


# This should fail or there should be at least a warning.
# Reported as https://github.com/rpm-software-management/dnf5/issues/1571
@xfail
Scenario: Replay installing a package with reason group while the group is not installed
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "Group",
                  "repo_id": "transaction-sr",
                  "group_id": "test-group"
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
  """
  Something like: Cannot install package with reason Group because 'test-group` is not installed.
  """


Scenario: Replay installing a group without package_types specified sets no package types
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Install",
                  "id": "test-group",
                  "reason": "User",
              }
          ],
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package    |
      | group-install | Test Group |
  And dnf5 transaction items for transaction "last" are
      | action        | package    | reason     | repository     |
      | Install       | test-group | User       | transaction-sr |
  And group state is
      | id         | package_types | packages | userinstalled |
      | test-group |               | top-a    | True          |


Scenario: Replay installing a group with package_types which are then used for upgrade
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Install",
                  "id": "test-group",
                  "package_types": "mandatory",
                  "reason": "User",
              }
          ],
          "version": "1.0"
      }
      """
  And I successfully execute dnf with args "replay ./transaction"
  And I drop repository "transaction-sr"
  And I use repository "transaction-sr-upgrade"
  # Only new mandatory packages are installed from test-group from transaction-sr-upgade,
  # it doesn't include new "default" package bottom-f.
 When I execute dnf with args "group upgrade test-group"
 Then the exit code is 0
 And Transaction is following
      | Action        | Package               |
      | install-group | bottom-e-1.0-1.x86_64 |
      | group-upgrade | Test Group            |
  And group state is
      | id         | package_types | packages        | userinstalled |
      | test-group | mandatory     | bottom-e, top-a | True          |
