Feature: Transaction store tests

Background:
Given I set working directory to "{context.dnf.tempdir}"
Given I use repository "transaction-sr"
  And I successfully execute dnf with args "install top-a-1.0"


Scenario: Store a transaction
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Install",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"mid-a1-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"bottom-a2-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"mid-a2-1.0-1.x86_64",
            "action":"Install",
            "reason":"Weak Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"bottom-a3-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"transaction-sr"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction with an invalid transaction ID
 When I execute dnf with args "history store 2"
 Then the exit code is 1
  And stderr is
      """
      No matching transaction ID found, exactly one required.
      """


Scenario: Store an upgrade transaction
Given I successfully execute dnf with args "upgrade top-a"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"bottom-a1-2.0-1.noarch",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"top-a-1:2.0-1.x86_64",
            "action":"Upgrade",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Replaced",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a reinstall transaction
Given I successfully execute dnf with args "reinstall top-a"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Reinstall",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Replaced",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a downgrade transaction
Given I successfully execute dnf with args "upgrade top-a"
Given I successfully execute dnf with args "downgrade top-a"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Downgrade",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"top-a-1:2.0-1.x86_64",
            "action":"Replaced",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a remove transaction
Given I successfully execute dnf with args "remove top-a"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"bottom-a2-1.0-1.x86_64",
            "action":"Remove",
            "reason":"Clean",
            "repo_id":"@System"
          },
          {
            "nevra":"bottom-a3-1.0-1.x86_64",
            "action":"Remove",
            "reason":"Clean",
            "repo_id":"@System"
          },
          {
            "nevra":"mid-a1-1.0-1.x86_64",
            "action":"Remove",
            "reason":"Clean",
            "repo_id":"@System"
          },
          {
            "nevra":"mid-a2-1.0-1.x86_64",
            "action":"Remove",
            "reason":"Clean",
            "repo_id":"@System"
          },
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Remove",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a reason change transaction
Given I successfully execute dnf with args "mark dependency top-a"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Reason Change",
            "reason":"Dependency",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction with a group install
Given I successfully execute dnf with args "install @test-group"
 When I execute dnf with args "history store 2"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-b-1.0-1.x86_64",
            "action":"Install",
            "reason":"Group",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"bottom-a1-2.0-1.noarch",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"top-a-1:2.0-1.x86_64",
            "action":"Upgrade",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Replaced",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "groups":[
          {
            "id":"test-group",
            "action":"Install",
            "reason":"User",
            "repo_id":"transaction-sr",
            "package_types":"mandatory, default, conditional"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction with a group remove
Given I successfully execute dnf with args "install @test-group"
  And I successfully execute dnf with args "remove @test-group"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-b-1.0-1.x86_64",
            "action":"Remove",
            "reason":"Group",
            "repo_id":"@System"
          }
        ],
        "groups":[
          {
            "id":"test-group",
            "action":"Remove",
            "reason":"User",
            "repo_id":"@System",
            "package_types":""
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction with a group upgrade
Given I successfully execute dnf with args "install @test-group"
Given I successfully execute dnf with args "install top-a-1.0"
Given I successfully execute dnf with args "upgrade @test-group"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-a-1:2.0-1.x86_64",
            "action":"Upgrade",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Replaced",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "groups":[
          {
            "id":"test-group",
            "action":"Upgrade",
            "reason":"User",
            "repo_id":"transaction-sr",
            "package_types":"mandatory, default, conditional"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction with an enviroment group install
Given I successfully execute dnf with args "install @test-env"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-c-2.0-1.x86_64",
            "action":"Install",
            "reason":"Group",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"mid-a2-2.0-1.x86_64",
            "action":"Upgrade",
            "reason":"Weak Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"mid-a2-1.0-1.x86_64",
            "action":"Replaced",
            "reason":"Weak Dependency",
            "repo_id":"@System"
          }
        ],
        "groups":[
          {
            "id":"test-env-group",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"transaction-sr",
            "package_types":"mandatory, default, conditional"
          }
        ],
        "environments":[
          {
            "id":"test-env",
            "action":"Install",
            "repo_id":"transaction-sr"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction with an environment group remove
Given I successfully execute dnf with args "install @test-env"
  And I successfully execute dnf with args "remove @test-env"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"bottom-a3-1.0-1.x86_64",
            "action":"Remove",
            "reason":"Clean",
            "repo_id":"@System"
          },
          {
            "nevra":"mid-a2-2.0-1.x86_64",
            "action":"Remove",
            "reason":"Clean",
            "repo_id":"@System"
          },
          {
            "nevra":"top-c-2.0-1.x86_64",
            "action":"Remove",
            "reason":"Group",
            "repo_id":"@System"
          }
        ],
        "groups":[
          {
            "id":"test-env-group",
            "action":"Remove",
            "reason":"Dependency",
            "repo_id":"@System",
            "package_types":""
          }
        ],
        "environments":[
          {
            "id":"test-env",
            "action":"Remove",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction with an environment group upgrade
Given I successfully execute dnf with args "install @test-env"
  And I successfully execute dnf with args "install top-c-1.0"
  And I successfully execute dnf with args "upgrade @test-env"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-c-2.0-1.x86_64",
            "action":"Upgrade",
            "reason":"Group",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"mid-a2-2.0-1.x86_64",
            "action":"Upgrade",
            "reason":"Weak Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"mid-a2-1.0-1.x86_64",
            "action":"Replaced",
            "reason":"Weak Dependency",
            "repo_id":"@System"
          },
          {
            "nevra":"top-c-1.0-1.x86_64",
            "action":"Replaced",
            "reason":"Group",
            "repo_id":"@System"
          }
        ],
        "groups":[
          {
            "id":"test-env-group",
            "action":"Upgrade",
            "reason":"Dependency",
            "repo_id":"transaction-sr",
            "package_types":"mandatory, default, conditional"
          }
        ],
        "environments":[
          {
            "id":"test-env",
            "action":"Upgrade",
            "repo_id":"transaction-sr"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction installing multiple installonly package versions
Given I successfully execute dnf with args "install installonly-1.0 installonly-2.0"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"installonly-1.0-1.x86_64",
            "action":"Install",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"installonly-2.0-1.x86_64",
            "action":"Install",
            "reason":"User",
            "repo_id":"transaction-sr"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction removing multiple installonly package versions
Given I successfully execute dnf with args "install installonly-1.0 installonly-2.0"
  And I successfully execute dnf with args "remove installonly-1.0 installonly-2.0"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"installonly-2.0-1.x86_64",
            "action":"Remove",
            "reason":"User",
            "repo_id":"@System"
          },
          {
            "nevra":"installonly-1.0-1.x86_64",
            "action":"Remove",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


# no dnf shell for dnf5
# https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Scenario: Store a transaction installing and removing an installonly package
Given I successfully execute dnf with args "install installonly-1.0"
  And I open dnf shell session
  And I execute in dnf shell "install installonly-2.0"
  And I execute in dnf shell "remove installonly-1.0"
  And I execute in dnf shell "run"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "action":"Install",
            "nevra":"installonly-2.0-1.x86_64",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "action":"Remove",
            "nevra":"installonly-1.0-1.x86_64",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction upgrading an installonly package
Given I successfully execute dnf with args "install installonly-1.0"
  And I successfully execute dnf with args "upgrade installonly"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"installonly-2.0-1.x86_64",
            "action":"Install",
            "reason":"User",
            "repo_id":"transaction-sr"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction obsoleting a package
Given I successfully execute dnf with args "install obsoleted-a-1.0"
  And I successfully execute dnf with args "upgrade obsoleted-a"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"obsoleting-x-2.0-1.x86_64",
            "action":"Install",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"obsoleted-a-1.0-1.x86_64",
            "action":"Replaced",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction obsoleting multiple packages
Given I successfully execute dnf with args "install obsoleted-a-1.0 obsoleted-b-1.0"
  And I successfully execute dnf with args "upgrade obsoleted-a obsoleted-b"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"obsoleting-x-2.0-1.x86_64",
            "action":"Install",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"obsoleting-y-2.0-1.x86_64",
            "action":"Install",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"obsoleted-a-1.0-1.x86_64",
            "action":"Replaced",
            "reason":"User",
            "repo_id":"@System"
          },
          {
            "nevra":"obsoleted-b-1.0-1.x86_64",
            "action":"Replaced",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction with an arch change
Given I successfully execute dnf with args "install archchange-1.0"
  And I successfully execute dnf with args "upgrade archchange"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"archchange-2.0-1.x86_64",
            "action":"Upgrade",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"archchange-1.0-1.noarch",
            "action":"Replaced",
            "reason":"User",
            "repo_id":"@System"
          }
        ],
        "version":"1.0"
      }
      """


# no dnf shell for dnf5
# https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Scenario: Store a transaction with multiple actions per NEVRA (removing a group and reinstalling its package while another package depends on it)
Given I successfully execute dnf with args "install @test-group supertop-b"
  And I open dnf shell session
  And I execute in dnf shell "remove @test-group"
  And I execute in dnf shell "reinstall top-b-1.0-1.x86_64"
  And I execute in dnf shell "run"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
          "groups":[
              {
                  "action":"Remove",
                  "id":"test-group",
                  "package_types":"conditional, default, mandatory",
              }
          ],
          "rpms":[
              {
                  "action":"Reason Change",
                  "nevra":"top-b-1.0-1.x86_64",
                  "reason":"Dependency",
                  "repo_id":"transaction-sr"
              },
              {
                  "action":"Reinstall",
                  "nevra":"top-b-1.0-1.x86_64",
                  "reason":"Group",
                  "repo_id":"transaction-sr"
              },
              {
                  "action":"Replaced",
                  "nevra":"top-b-1.0-1.x86_64",
                  "reason":"Group",
                  "repo_id":"@System"
              }
          ],
          "version":"1.0"
      }
      """


# no dnf shell for dnf5
# https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Scenario: Store a transaction with removing a group and reinstalling its package (unlike the scenario above, the reason of the package stays unchanged)
Given I successfully execute dnf with args "install @test-group"
  And I open dnf shell session
  And I execute in dnf shell "remove @test-group"
  And I execute in dnf shell "reinstall top-b-1.0-1.x86_64"
  And I execute in dnf shell "run"
 When I execute dnf with args "history store last"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to ./transaction.
      """
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
          "groups":[
              {
                  "action":"Remove",
                  "id":"test-group",
                  "package_types":"conditional, default, mandatory",
              }
          ],
          "rpms":[
              {
                  "action":"Reinstall",
                  "nevra":"top-b-1.0-1.x86_64",
                  "reason":"Group",
                  "repo_id":"transaction-sr"
              },
              {
                  "action":"Replaced",
                  "nevra":"top-b-1.0-1.x86_64",
                  "reason":"Group",
                  "repo_id":"@System"
              }
          ],
          "version":"1.0"
      }
      """


Scenario: Store a transaction with specifying the output file
 When I execute dnf with args "history store last -o {context.dnf.tempdir}/out"
 Then the exit code is 0
  And stderr is
      """
      Transaction saved to {context.dnf.tempdir}/out.
      """
  And file "/{context.dnf.tempdir}/out/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Install",
            "reason":"User",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"mid-a1-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"bottom-a2-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"mid-a2-1.0-1.x86_64",
            "action":"Install",
            "reason":"Weak Dependency",
            "repo_id":"transaction-sr"
          },
          {
            "nevra":"bottom-a3-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"transaction-sr"
          }
        ],
        "version":"1.0"
      }
      """


Scenario: Store a transaction to a file that already exists and --assumeyes
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      STUFF
      """
 When I execute dnf with args "history store last -y"
 Then the exit code is 0
  And stderr is
      """
      File "./transaction/transaction.json" already exists, it will be overwritten.
      Transaction saved to ./transaction.
      """


Scenario: Store a transaction to a file that already exists and --assumeno
Given I create file "/{context.dnf.tempdir}/transaction/transaction.json" with
      """
      STUFF
      """
 When I execute dnf with args "history store last --assumeno"
 Then the exit code is 1
  And stderr is
      """
      File "./transaction/transaction.json" already exists, it will be overwritten.
      Operation aborted by the user.
      """
