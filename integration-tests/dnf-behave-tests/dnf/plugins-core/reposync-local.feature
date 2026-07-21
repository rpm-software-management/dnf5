Feature: Tests for reposync command with local repository


Scenario: Base functionality of reposync on local repository
  Given I use repository "dnf-ci-thirdparty-updates"
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir}"
   Then the exit code is 0
    And stderr contains " CQRlib-extension-0:1\.6-2\.src\s.*"
    And stderr contains " CQRlib-extension-0:1\.6-2\.x86_64\s.*"
    And stderr contains " SuperRipper-0:1\.2-1\.src\s.*"
    And stderr contains " SuperRipper-0:1\.2-1\.x86_64\s.*"
    And stderr contains " SuperRipper-0:1\.3-1\.src\s.*"
    And stderr contains " SuperRipper-0:1\.3-1\.x86_64\s.*"
    And the files "{context.dnf.tempdir}/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm" and "{context.dnf.fixturesdir}/repos/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm" do not differ
