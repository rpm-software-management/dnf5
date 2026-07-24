Feature: Transaction replay invalid json tests

Background:
Given I set working directory to "{context.dnf.tempdir}"
Given I use repository "transaction-sr"


Scenario: Replay a non-existant transaction dir
 When I execute dnf with args "replay nonexistent"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      cannot open file: (2) - No such file or directory [nonexistent/transaction.json]
      """


Scenario: Replay a broken json
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "version": "1.0"
          "missing_comma": 1
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Error during transaction replay JSON parsing : object value separator ',' expected.
      """


Scenario: Replay a json missing version
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": []
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Missing key "version".
      """


Scenario: Replay a json wrong version type
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "version": 1
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Unexpected version format: "1", supported version is "1.0".
      """


Scenario: Replay a json invalid major version characters
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "version": "a.1"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Incompatible major version: "a", supported major version is "1".
      """


Scenario: Replay a json invalid minor version characters
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "version": "1.a"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Invalid minor version: "a", number expected.
      """


Scenario: Replay a json incompatible major version
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "version": "5.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Incompatible major version: "5", supported major version is "1".
      """


Scenario: Replay a json wrong packages type
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": "hi",
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Unexpected type of "rpms", array expected.
      """


Scenario: Replay a json with missing action
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "user",
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
      Cannot parse file: './transaction/transaction.json': Missing object key "action" in an rpm.
      """


Scenario: Replay a json with missing nevra
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "reason": "user",
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
      Cannot parse file: './transaction/transaction.json': Either "nevra" or "package_path" object key is required in an rpm.
      """


Scenario: Replay a json with missing reason
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "top-a-1:1.0-1.x86_64",
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
      Cannot parse file: './transaction/transaction.json': Missing object key "reason" in an rpm.
      """


Scenario: Replay a json with invalid action
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Fixxit",
                  "nevra": "top-a-1:1.0-1.x86_64",
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
      Invalid transaction item action: Fixxit
      """


Scenario: Replay a json unparseable package nevra (the code cannot distinguish invalid nevra atm.)
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "wakaka",
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
      Cannot parse file: './transaction/transaction.json': Cannot parse NEVRA for rpm "wakaka".
      """


Scenario: Replay a json with invalid reason
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "top-a-1:1.0-1.x86_64",
                  "reason": "dumb",
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
      Invalid transaction item reason: dumb
      """


Scenario: Replay a json wrong groups type
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": "hi",
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Unexpected type of "groups", array expected.
      """


Scenario: Replay a json with missing group id
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Invalid"
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
      Cannot parse file: './transaction/transaction.json': Missing object key "id" in a group.
      """


Scenario: Replay a json with missing group action
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "id": "dummy",
                  "package_types": "mandatory",
                  "packages": []
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
      Cannot parse file: './transaction/transaction.json': Missing object key "action" in a group.
      """


Scenario: Replay a json with invalid group action
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Invalid",
                  "id": "dummy",
                  "package_types": "mandatory",
                  "packages": [],
                  "reason": "User"
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
      Invalid transaction item action: Invalid
      """


Scenario: Replay a json with invalid group package_types
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Install",
                  "id": "dummy",
                  "package_types": "aaa, default",
                  "reason": "User"
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
      Invalid package type: aaa
      """


Scenario: Replay a json wrong environments type
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": "hi",
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Unexpected type of "environments", array expected.
      """


Scenario: Replay a json with missing environment id
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "action": "Invalid"
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
      Cannot parse file: './transaction/transaction.json': Missing object key "id" in an environment.
      """


Scenario: Replay a json with missing environment action
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "id": "dummy"
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
      Cannot parse file: './transaction/transaction.json': Missing object key "action" in an environment.
      """


Scenario: Replay a json with invalid environment action
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "action": "Invalid",
                  "id": "dummy",
                  "package_types": "mandatory"
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
      Invalid transaction item action: Invalid
      """
