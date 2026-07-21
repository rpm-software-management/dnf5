Feature: Test for --repofrompath option

Scenario: I can use --repofrompath to add a repository
  When I execute dnf with args "repo list --repofrompath=NEW_REPO,THE_PATH"
  Then the exit code is 0
   And stdout is
   """
   repo id  repo name
   NEW_REPO NEW_REPO
   """

Scenario: The packages from --repofrompath repository are available
 Given I copy repository "simple-base" for modification
  When I execute dnf with args "repoquery --repofrompath=NEW_REPO,{context.dnf.tempdir}/repos/simple-base/"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       dedalo-0:1.0-1.fc29.src
       dedalo-0:1.0-1.fc29.x86_64
       labirinto-0:1.0-1.fc29.src
       labirinto-0:1.0-1.fc29.x86_64
       vagare-0:1.0-1.fc29.src
       vagare-0:1.0-1.fc29.x86_64
       """

Scenario: I can use --repofrompath multiple times
  When I execute dnf with args "repo list --repofrompath=NEW_REPO,THE_PATH --repofrompath=ANOTHER_ID,ANOTHER_PATH"
  Then the exit code is 0
   And stdout is
   """
   repo id    repo name
   ANOTHER_ID ANOTHER_ID
   NEW_REPO   NEW_REPO
   """

Scenario: I can fine tune --repofrompath repos with --setopt
  When I execute dnf with args "repo list --repofrompath=NEW_REPO,THE_PATH --setopt=NEW_REPO.name='Real name'"
  Then the exit code is 0
   And stdout is
   """
   repo id  repo name
   NEW_REPO Real name
   """

Scenario: Variables in the path are substituted
 Given I copy repository "simple-base" for modification
   And I execute "mv {context.dnf.tempdir}/repos/simple-base {context.dnf.tempdir}/repos/simple-base-RLS"
  When I execute dnf with args "repoquery --releasever=RLS --repofrompath=NEW_REPO,{context.dnf.tempdir}/repos/simple-base-\$releasever/"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       dedalo-0:1.0-1.fc29.src
       dedalo-0:1.0-1.fc29.x86_64
       labirinto-0:1.0-1.fc29.src
       labirinto-0:1.0-1.fc29.x86_64
       vagare-0:1.0-1.fc29.src
       vagare-0:1.0-1.fc29.x86_64
       """

Scenario: Variables in the repo id are substituted
  When I execute dnf with args "repo list --releasever=RLS --repofrompath=NEW_REPO_\${{releasever}},THE_PATH"
  Then the exit code is 0
   And stdout is
   """
   repo id      repo name
   NEW_REPO_RLS NEW_REPO_RLS
   """

# https://github.com/rpm-software-management/dnf5/issues/794
Scenario: Repofrompath does not modify existing repo with the same id
 Given I use repository "simple-base"
  When I execute dnf with args "repo list --repofrompath=simple-base,/the/path"
  Then the exit code is 1
   And stderr is
   """
   Failed to create repo "simple-base": Id is present more than once in the configuration
   """

# https://github.com/rpm-software-management/dnf5/issues/793
Scenario: Repofrompath repos are enabled even with --disablerepo=*
  When I execute dnf with args "repo list --disablerepo=* --repofrompath=NEW_REPO,THE_PATH"
  Then the exit code is 0
   And stdout is
   """
   repo id  repo name
   NEW_REPO NEW_REPO
   """
