Feature: Transaction replay tests

Background:
Given I set working directory to "{context.dnf.tempdir}"
  And I use repository "transaction-sr"
  And I successfully execute dnf with args "install top-a-1.0"


Scenario: Replay a transaction installing a nonexistent package
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
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
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot perform Install action, no match for: does-not-exist-1.0-1.noarch.
      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
      """


Scenario: Replay a transaction installing a nonexistent package version
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "top-a-1:3.0-1.x86_64",
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
      Cannot perform Install action, no match for: top-a-1:3.0-1.x86_64.
      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
      """


Scenario: Replaying an already installed package
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a2-0:1.0-1.x86_64",
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
      Package "bottom-a2-0:1.0-1.x86_64" is already installed.
      """


Scenario: Replay a transaction upgrading a nonexistent package
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Upgrade",
                  "nevra": "does-not-exist-2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "does-not-exist-1.0-1.x86_64",
                  "reason": "User",
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
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot perform Install action, no match for: does-not-exist-2.0-1.x86_64.
      Cannot perform Remove action, no match for: does-not-exist-1.0-1.x86_64.
      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
      """


Scenario: Replay a transaction upgrading to a nonexistent package version
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Upgrade",
                  "nevra": "top-a-1:3.0-1.x86_64",
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
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot perform Install action, no match for: top-a-1:3.0-1.x86_64.
      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
      """


Scenario: Replay a transaction upgrading from a not-installed package version
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Upgrade",
                  "nevra": "top-a-1:2.0-1.x86_64",
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
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot perform Remove action because 'top-a-1:2.0-1.x86_64' is installed in a different version: 'top-a-1:1.0-1.x86_64'.
      Extra package 'bottom-a1-2.0-1.noarch' (with action 'Install') which is not present in the stored transaction was pulled into the transaction.

      Extra package 'top-a-1:1.0-1.x86_64' (with action 'Replaced') which is not present in the stored transaction was pulled into the transaction.

      You can try to add to command line:
        --ignore-extras to allow extra packages in the transaction
        --ignore-installed to allow mismatches between installed and stored transaction packages. This can result in an empty transaction because among other things the option can ignore failing Remove actions.
      """


Scenario: Replay a transaction removing a nonexistent package
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Remove",
                  "nevra": "does-not-exist-1.0-1.x86_64",
                  "reason": "User",
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
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot perform Remove action, no match for: does-not-exist-1.0-1.x86_64.
      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
      """


Scenario: Replay a transaction removing a package that is not installed
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Remove",
                  "nevra": "top-c-1.0-1.x86_64",
                  "reason": "User",
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
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot perform Remove action for Package 'top-c-1.0-1.x86_64' because it is not installed.
      You can try to add to command line:
        --ignore-installed to allow mismatches between installed and stored transaction packages. This can result in an empty transaction because among other things the option can ignore failing Remove actions.
      """


Scenario: Replay a transaction that pulls in an extra package
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
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
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Extra package 'bottom-a1-2.0-1.noarch' (with action 'Install') which is not present in the stored transaction was pulled into the transaction.

      You can try to add to command line:
        --ignore-extras to allow extra packages in the transaction
      """


Scenario: Replay a transaction with a dependency conflict
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "bottom-a1-1.0-1.noarch",
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
      Problem: package top-a-1:2.0-1.x86_64 from transaction-sr requires bottom-a1 = 2.0-1, but none of the providers can be installed
        - cannot install both bottom-a1-2.0-1.noarch from transaction-sr and bottom-a1-1.0-1.noarch from transaction-sr
        - conflicting requests
      You can try to add to command line:
        --skip-broken to skip uninstallable packages
      """


Scenario: Replay a transaction with a broken dependency
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "rpms": [
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
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      Problem: conflicting requests
        - nothing provides nonexistent needed by broken-dep-1.0-1.x86_64 from transaction-sr
      You can try to add to command line:
        --skip-broken to skip uninstallable packages
      """


Scenario: Replay a transaction installing a nonexistent group
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Install",
                  "id": "nonexistent",
                  "package_types": "conditional, default, mandatory",
                  "reason": "User"
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
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      No match for argument: nonexistent
      Extra package 'bottom-a1-2.0-1.noarch' (with action 'Install') which is not present in the stored transaction was pulled into the transaction.

      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
        --ignore-extras to allow extra packages in the transaction

      """


Scenario: Replay a transaction removing a nonexistent group
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Remove",
                  "id": "nonexistent",
                  "package_types": "conditional, default, mandatory",
                  "reason": "User",
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
      Cannot perform Remove action for Group 'nonexistent' because it is not installed.
      No groups to remove for argument: nonexistent
      You can try to add to command line:
        --ignore-installed to allow mismatches between installed and stored transaction packages. This can result in an empty transaction because among other things the option can ignore failing Remove actions.
      """


Scenario: Replay a transaction upgrading a nonexistent group
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "groups": [
              {
                  "action": "Upgrade",
                  "id": "nonexistent",
                  "package_types": "conditional, default, mandatory",
                  "reason": "User",
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
      Cannot perform Upgrade action for Group 'nonexistent' because it is not installed.
      No match for argument: nonexistent
      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
        --ignore-installed to allow mismatches between installed and stored transaction packages. This can result in an empty transaction because among other things the option can ignore failing Remove actions.
      """


Scenario: Replay a transaction upgrading an installed nonexistent group
Given I successfully execute dnf with args "install @test-group"
  And I drop repository "transaction-sr"
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
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
          "version": "1.0"
      }
      """
 When I execute dnf with args "replay ./transaction"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      Packages for argument 'test-group' installed, but not available.
      """


Scenario: Replay a transaction installing a nonexistent environment
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "action": "Install",
                  "id": "nonexistent",
                  "package_types": "conditional, default, mandatory"
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
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      No match for argument: nonexistent
      Extra package 'bottom-a1-2.0-1.noarch' (with action 'Install') which is not present in the stored transaction was pulled into the transaction.

      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
        --ignore-extras to allow extra packages in the transaction
      """


Scenario: Replay a transaction removing a nonexistent environment
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "action": "Remove",
                  "id": "nonexistent",
                  "package_types": "conditional, default, mandatory"
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
      Cannot perform Remove action for Environment 'nonexistent' because it is not installed.
      No groups to remove for argument: nonexistent
      You can try to add to command line:
        --ignore-installed to allow mismatches between installed and stored transaction packages. This can result in an empty transaction because among other things the option can ignore failing Remove actions.
      """


Scenario: Replay a transaction upgrading a nonexistent environment
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "action": "Upgrade",
                  "id": "nonexistent",
                  "package_types": "conditional, default, mandatory"
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
      Cannot perform Upgrade action for Environment 'nonexistent' because it is not installed.
      No match for argument: nonexistent
      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
        --ignore-installed to allow mismatches between installed and stored transaction packages. This can result in an empty transaction because among other things the option can ignore failing Remove actions.
      """


Scenario: Replay a transaction with multiple errors
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      {
          "environments": [
              {
                  "action": "Install",
                  "id": "dummy"
              }
          ],
          "groups": [
              {
                  "reason": "User",
                  "action": "Upgrade"
              }
          ],
          "rpms": [
              {
                  "action": "Install",
                  "nevra": "top-a-1:3.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Upgrade",
                  "nevra": "top-b-2.0-1.x86_64",
                  "reason": "User",
                  "repo_id": "transaction-sr"
              },
              {
                  "action": "Replaced",
                  "nevra": "top-b-1.0-1.x86_64",
                  "reason": "User",
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
      <REPOSYNC>
      Failed to resolve the transaction:
      Cannot parse file: './transaction/transaction.json': Missing object key "id" in a group.
      """
  #TODO(amatej): we could potentially improve the behavior by reporting all the issues at once like dnf4:
  #Error: The following problems occurred while replaying the transaction from file "{context.dnf.tempdir}/transaction.json":
  #  Cannot find rpm nevra "top-a-1:3.0-1.x86_64".
  #  Cannot find rpm nevra "top-b-2.0-1.x86_64".
  #  Package nevra "top-b-1.0-1.x86_64" not installed for action "Replaced".
  #  Missing object key "id" in a group.
  #  Missing object key "package_types" in an environment.
