Feature: Transaction --store tests

Background:
Given I set working directory to "{context.dnf.tempdir}"
  And I use repository "transaction-sr"


Scenario: Store an install transaction
  When I execute dnf with args "install top-a-1.0 --store ./transaction"
 Then the exit code is 0
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-a-1:1.0-1.x86_64",
            "action":"Install",
            "reason":"User",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/top-a-1.0-1.x86_64.rpm"
          },
          {
            "nevra":"mid-a1-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/mid-a1-1.0-1.x86_64.rpm"
          },
          {
            "nevra":"bottom-a2-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/bottom-a2-1.0-1.x86_64.rpm"
          },
          {
            "nevra":"mid-a2-1.0-1.x86_64",
            "action":"Install",
            "reason":"Weak Dependency",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/mid-a2-1.0-1.x86_64.rpm"
          },
          {
            "nevra":"bottom-a3-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/bottom-a3-1.0-1.x86_64.rpm"
          }
        ],
        "version":"1.0"
      }
      """
   When I execute "ls {context.dnf.tempdir}/transaction/"
   Then stdout is
        """
        packages
        transaction.json
        """
   When I execute "ls {context.dnf.tempdir}/transaction/packages"
   Then stdout is
        """
        bottom-a2-1.0-1.x86_64.rpm
        bottom-a3-1.0-1.x86_64.rpm
        mid-a1-1.0-1.x86_64.rpm
        mid-a2-1.0-1.x86_64.rpm
        top-a-1.0-1.x86_64.rpm
        """


Scenario: Store a remove transaction
Given I successfully execute dnf with args "install top-a-1.0"
 When I execute dnf with args "remove top-a --store ./transaction"
 Then the exit code is 0
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
   When I execute "ls {context.dnf.tempdir}/transaction/"
   Then stdout is
        """
        transaction.json
        """


Scenario: Store a group install transaction
 When I execute dnf with args "install @test-group --store ./transaction"
 Then the exit code is 0
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"top-b-1.0-1.x86_64",
            "action":"Install",
            "reason":"Group",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/top-b-1.0-1.x86_64.rpm"
          },
          {
            "nevra":"top-a-1:2.0-1.x86_64",
            "action":"Install",
            "reason":"Group",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/top-a-2.0-1.x86_64.rpm"
          },
          {
            "nevra":"mid-a1-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/mid-a1-1.0-1.x86_64.rpm"
          },
          {
            "nevra":"bottom-a1-2.0-1.noarch",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/bottom-a1-2.0-1.noarch.rpm"
          },
          {
            "nevra":"bottom-a2-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/bottom-a2-1.0-1.x86_64.rpm"
          },
          {
            "nevra":"mid-a2-2.0-1.x86_64",
            "action":"Install",
            "reason":"Weak Dependency",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/mid-a2-2.0-1.x86_64.rpm"
          },
          {
            "nevra":"bottom-a3-1.0-1.x86_64",
            "action":"Install",
            "reason":"Dependency",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_path":".\/packages\/bottom-a3-1.0-1.x86_64.rpm"
          }
        ],
        "groups":[
          {
            "id":"test-group",
            "action":"Install",
            "reason":"User",
            "group_path":".\/comps\/groups\/test-group.xml",
            "repo_id":"@stored_transaction(transaction-sr)",
            "package_types":"mandatory, default, conditional"
          }
        ],
        "version":"1.0"
      }
      """
   When I execute "ls {context.dnf.tempdir}/transaction/"
   Then stdout is
        """
        comps
        packages
        transaction.json
        """
   When I execute "ls {context.dnf.tempdir}/transaction/packages"
   Then stdout is
        """
        bottom-a1-2.0-1.noarch.rpm
        bottom-a2-1.0-1.x86_64.rpm
        bottom-a3-1.0-1.x86_64.rpm
        mid-a1-1.0-1.x86_64.rpm
        mid-a2-2.0-1.x86_64.rpm
        top-a-2.0-1.x86_64.rpm
        top-b-1.0-1.x86_64.rpm
        """
   When I execute "ls {context.dnf.tempdir}/transaction/comps/groups"
   Then stdout is
        """
        test-group.xml
        """


Scenario: Store a group remove transaction
Given I execute dnf with args "install @test-group"
 When I execute dnf with args "remove @test-group --store ./transaction"
 Then the exit code is 0
  And file "/{context.dnf.tempdir}/transaction/transaction.json" contents is
      """
      {
        "rpms":[
          {
            "nevra":"bottom-a1-2.0-1.noarch",
            "action":"Remove",
            "reason":"Clean",
            "repo_id":"@System"
          },
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
            "nevra":"mid-a2-2.0-1.x86_64",
            "action":"Remove",
            "reason":"Clean",
            "repo_id":"@System"
          },
          {
            "nevra":"top-a-1:2.0-1.x86_64",
            "action":"Remove",
            "reason":"Group",
            "repo_id":"@System"
          },
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
            "group_path":".\/comps\/groups\/test-group.xml",
            "repo_id":"@System",
            "package_types":""
          }
        ],
        "version":"1.0"
      }
      """
   When I execute "ls {context.dnf.tempdir}/transaction/"
   Then stdout is
        """
        comps
        transaction.json
        """
   When I execute "ls {context.dnf.tempdir}/transaction/comps/groups"
   Then stdout is
        """
        test-group.xml
        """


# https://github.com/rpm-software-management/dnf5/issues/2160
Scenario: stored packages are not removed by following transaction even with keepcache=false
   When I execute dnf with args "install top-a-1.0 --store ./transaction"
   Then file "/{context.dnf.tempdir}/transaction/packages/top-a-1.0-1.x86_64.rpm" exists
   When I execute dnf with args "install top-a-1.0 --setopt=keepcache=false"
   Then the exit code is 0
    And Transaction contains
        | Action        | Package              |
        | install       | top-a-1:1.0-1.x86_64 |
    And file "/{context.dnf.tempdir}/transaction/packages/top-a-1.0-1.x86_64.rpm" exists


# https://github.com/rpm-software-management/dnf5/issues/2160
Scenario: stored packages with keepcache=false are not removed by following transaction even with keepcache=false
   When I execute dnf with args "install top-a-1.0 --store ./transaction --setopt=keepcache=false"
   Then file "/{context.dnf.tempdir}/transaction/packages/top-a-1.0-1.x86_64.rpm" exists
   When I execute dnf with args "install top-a-1.0 --setopt=keepcache=false"
   Then the exit code is 0
    And Transaction contains
        | Action        | Package              |
        | install       | top-a-1:1.0-1.x86_64 |
    And file "/{context.dnf.tempdir}/transaction/packages/top-a-1.0-1.x86_64.rpm" exists


Scenario: storing transaction uses already cached packages
  Given I use repository "transaction-sr" as http
    And I execute dnf with args "install top-a-1.0 --downloadonly"
    And I start capturing outbound HTTP requests
   When I execute dnf with args "install top-a-1.0 --store ./transaction"
   Then file "/{context.dnf.tempdir}/transaction/packages/top-a-1.0-1.x86_64.rpm" exists
    # Both needed metadata and packages are already cached, nothing should be redownloaded
    And HTTP log is empty
