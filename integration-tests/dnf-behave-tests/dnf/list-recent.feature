Feature: Test dnf list --recent


Background: prepare repository with recent package labirinto
  Given I copy repository "simple-base" for modification
    # rebuild all packages in simple-base repo with buildtime 10 days ago
    And I execute "SOURCE_DATE_EPOCH=$(date --date='-10 day' +%s) rpmbuild -rb --define "_rpmdir ." --define "use_source_date_epoch_as_buildtime 1" /{context.dnf.repos[simple-base].path}/src/*.src.rpm" in "{context.dnf.repos[simple-base].path}"
    # rebuild labirinto package with current buildtime
    And I execute "SOURCE_DATE_EPOCH=$(date +%s) rpmbuild -rb --define "_rpmdir ." --define "use_source_date_epoch_as_buildtime 1" /{context.dnf.repos[simple-base].path}/src/labirinto*.src.rpm" in "{context.dnf.repos[simple-base].path}"
    # remove source rpms not to interfere with list outputs
    And I execute "rm -rf src" in "{context.dnf.repos[simple-base].path}"
    And I generate repodata for repository "simple-base"
    And I use repository "simple-base"


Scenario: dnf list --recent
   When I execute dnf with args "list --recent"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Recently added packages
        labirinto\.x86_64\s+1\.0-1\.fc29\s+simple-base
        """


Scenario: dnf list --recent --json
   When I execute dnf with args "list --recent --json"
   Then the exit code is 0
    And stdout json matches
        """
        {
        "recently_added_packages": [
          { "name": "labirinto", "arch": "x86_64", "evr": "1.0-1.fc29", "repository": "simple-base" }
        ]
        }
        """


Scenario: dnf list package that is not recently added
   # make sure that vagare package is present
   When I execute dnf with args "list vagare"
   Then the exit code is 0
   # but was not recently added
   When I execute dnf with args "list --recent vagare"
   Then the exit code is 1
    And stdout is empty
    And stderr is
        """
        <REPOSYNC>
        No matching packages to list
        """
