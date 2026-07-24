Feature: Tests for dnf repoquery --recent option


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
    # Rebuild packages are not signed, we have to use --no-gpgchecks
    And I successfully execute dnf with args "install --no-gpgchecks labirinto vagare"


Scenario: dnf repoquery --recent vagare (when there's no such recent pkg)
   When I execute dnf with args "repoquery --recent vagare"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is empty


Scenario: dnf repoquery --recent labirinto (when recent pkg exists)
   When I execute dnf with args "repoquery --recent labirinto"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        labirinto-0:1.0-1.fc29.x86_64
        """


Scenario: dnf repoquery --recent --installed labirinto (when recent pkg exists)
   When I execute dnf with args "repoquery --recent --installed"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        labirinto-0:1.0-1.fc29.x86_64
        """
