Feature: Tests for reposync command


Scenario: Base functionality of reposync
  Given I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir}"
   Then the exit code is 0
    And stderr contains " SuperRipper-0:1\.2-1\."
    And stderr contains " SuperRipper-0:1\.3-1\."
    And the files "{context.dnf.tempdir}/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm" and "{context.dnf.fixturesdir}/repos/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm" do not differ


Scenario: Reposync with --newest-only option
  Given I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --newest-only"
   Then the exit code is 0
    And stderr contains " SuperRipper-0:1\.3-1\."
    And stderr does not contain " SuperRipper-0:1\.2-1\."


@xfail
# --downloadcomps is not supported in dnf5
@bz1653126 @bz1676726
Scenario: Reposync with --downloadcomps option
  Given I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --downloadcomps"
   Then the exit code is 0
    And stdout contains "comps.xml for repository dnf-ci-thirdparty-updates saved"
    And the files "{context.dnf.tempdir}/dnf-ci-thirdparty-updates/comps.xml" and "{context.dnf.fixturesdir}/repos/dnf-ci-thirdparty-updates/repodata/comps.xml" do not differ
   When I execute "createrepo_c --no-database --simple-md-filenames --groupfile comps.xml ." in "{context.dnf.tempdir}/dnf-ci-thirdparty-updates"
   Then the exit code is 0
  Given I configure a new repository "testrepo" with
        | key             | value                                           |
        | baseurl         | {context.dnf.tempdir}/dnf-ci-thirdparty-updates |
    And I drop repository "dnf-ci-thirdparty-updates"
   When I execute dnf with args "group list"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
   """
   Available Groups:
      DNF-CI-Testgroup
   """


@xfail
# --downloadcomps is not supported in dnf5
@bz1676726
Scenario: Reposync with --downloadcomps option (comps.xml in repo does not exist)
  Given I use repository "dnf-ci-rich" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --downloadcomps"
   Then the exit code is 0
    And stdout does not contain "comps.xml for repository dnf-ci-rich saved"


@xfail
# --downloadcomps is not supported in dnf5
@bz1895059
Scenario: Reposync with --downloadcomps option (the comps.xml in repodata is not compressed)
  Given I copy repository "dnf-ci-thirdparty-updates" for modification
    And I execute "modifyrepo_c --remove group_gz /{context.dnf.repos[dnf-ci-thirdparty-updates].path}/repodata"
    And I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --downloadcomps"
   Then the exit code is 0
    And the files "{context.dnf.tempdir}/dnf-ci-thirdparty-updates/comps.xml" and "{context.dnf.fixturesdir}/repos/dnf-ci-thirdparty-updates/repodata/comps.xml" do not differ


@xfail
# --downloadcomps is not supported in dnf5
@bz1676726
Scenario: Reposync with --downloadcomps and --metadata-path options
  Given I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --metadata-path={context.dnf.tempdir}/downloadedmetadata --downloadcomps"
   Then the exit code is 0
    And stdout contains "comps.xml for repository dnf-ci-thirdparty-updates saved"
    And the files "{context.dnf.tempdir}/downloadedmetadata/dnf-ci-thirdparty-updates/comps.xml" and "{context.dnf.fixturesdir}/repos/dnf-ci-thirdparty-updates/repodata/comps.xml" do not differ


Scenario: Reposync with --download-metadata option
  Given I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --download-metadata"
   Then the exit code is 0
  Given I configure a new repository "testrepo" with
        | key             | value                                           |
        | baseurl         | {context.dnf.tempdir}/dnf-ci-thirdparty-updates |
    And I drop repository "dnf-ci-thirdparty-updates"
   When I execute dnf with args "group list"
   Then the exit code is 0
    And stdout is
   """
   ID                   Name             Installed
   dnf-ci-testgroup     DNF-CI-Testgroup        no
   """


# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
@bz1714788
Scenario: Reposync downloads packages from all streams of modular repository
  Given I use repository "dnf-ci-fedora-modular" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir}"
   Then the exit code is 0
    And file "//{context.dnf.tempdir}/dnf-ci-fedora-modular/x86_64/nodejs-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-fedora-modular/x86_64/nodejs-10.11.0-1.module_2200+adbac02b.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-fedora-modular/x86_64/nodejs-11.0.0-1.module_2311+8d497411.x86_64.rpm" exists


# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
@bz1714788
Scenario: Reposync downloads packages from all streams of modular repository even if the module is disabled
  Given I use repository "dnf-ci-fedora-modular" as http
   When I execute dnf with args "module disable nodejs"
   Then the exit code is 0
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir}"
   Then the exit code is 0
    And file "//{context.dnf.tempdir}/dnf-ci-fedora-modular/x86_64/nodejs-8.11.4-1.module_2030+42747d40.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-fedora-modular/x86_64/nodejs-10.11.0-1.module_2200+adbac02b.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-fedora-modular/x86_64/nodejs-11.0.0-1.module_2311+8d497411.x86_64.rpm" exists


@bz1750273
Scenario: Reposync respects excludes
  Given I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --setopt=excludepkgs=SuperRipper"
   Then the exit code is 0
    And stderr contains " CQRlib-extension-0:1\.6-2\.src"
    And stderr contains " CQRlib-extension-0:1\.6-2\.x86_64"
    And stderr does not contain "SuperRipper"
   When I execute "ls {context.dnf.tempdir}/dnf-ci-thirdparty-updates/x86_64/"
   Then stdout is
        """
        CQRlib-extension-1.6-2.x86_64.rpm
        """
   When I execute "ls {context.dnf.tempdir}/dnf-ci-thirdparty-updates/src/"
   Then stdout is
        """
        CQRlib-extension-1.6-2.src.rpm
        """


@bz1750273
Scenario: Reposync respects includes
  Given I use repository "dnf-ci-fedora" as http
  When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --arch=noarch --setopt=includepkgs=abcde"
   Then the exit code is 0
    And stderr contains "abcde-0:2.9.2-1.fc29.noarch"
   When I set LC_ALL to "C.UTF-8"
    And I execute "find | sort" in "{context.dnf.tempdir}"
   Then stdout is
    """
    .
    ./dnf-ci-fedora
    ./dnf-ci-fedora/noarch
    ./dnf-ci-fedora/noarch/abcde-2.9.2-1.fc29.noarch.rpm
    """


# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Scenario: Reposync respects excludes, but not modular excludes
  Given I use repository "dnf-ci-fedora-modular" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --setopt=includepkgs=postgresql"
   Then the exit code is 0
    And stderr contains " postgresql-0:6\.1-1\."
    And stderr contains " postgresql-0:9\.6\.8-1\."
    And stderr does not contain "ninja"
    And stderr does not contain "nodejs"
   When I execute "ls {context.dnf.tempdir}/dnf-ci-fedora-modular/x86_64/"
   Then stdout is
        """
        postgresql-6.1-1.module_2514+aa9aadc5.x86_64.rpm
        postgresql-9.6.8-1.module_1710+b535a823.x86_64.rpm
        """
   When I execute "ls {context.dnf.tempdir}/dnf-ci-fedora-modular/src/"
   Then stdout is
        """
        postgresql-6.1-1.module_2514+aa9aadc5.src.rpm
        postgresql-9.6.8-1.module_1710+b535a823.src.rpm
        """


Scenario: Reposync downloads packages and removes packages that are not part of repo anymore
  Given I use repository "setopt.ext"
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir}"
   Then the exit code is 0
    And file "//{context.dnf.tempdir}/setopt.ext/x86_64/wget-1.0-1.fc29.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/setopt.ext/src/wget-1.0-1.fc29.src.rpm" exists
    And file "//{context.dnf.tempdir}/setopt.ext/x86_64/flac-1.0-1.fc29.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/setopt.ext/src/flac-1.0-1.fc29.src.rpm" exists
    And file "//{context.dnf.tempdir}/setopt.ext/x86_64/flac-libs-1.0-1.fc29.x86_64.rpm" exists
  Given I configure repository "setopt.ext" with
        | key             | value                               |
        | baseurl         | {context.scenario.repos_location}/setopt |
    # The following two steps generate repodata for the repository without configuring it
    And I use repository "setopt"
    And I drop repository "setopt"
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --refresh --delete"
   Then the exit code is 0
    And file "//{context.dnf.tempdir}/setopt.ext/x86_64/wget-1.0-1.fc29.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/setopt.ext/src/wget-1.0-1.fc29.src.rpm" exists
    And file "//{context.dnf.tempdir}/setopt.ext/x86_64/flac-1.0-1.fc29.x86_64.rpm" does not exist
    And file "//{context.dnf.tempdir}/setopt.ext/src/flac-1.0-1.fc29.src.rpm" does not exist
    And file "//{context.dnf.tempdir}/setopt.ext/x86_64/flac-libs-1.0-1.fc29.x86_64.rpm" does not exist


@bz1774103
Scenario: Reposync --delete does not immediately delete downloaded content when multiple repositories are synced
  Given I use repository "reposync" as http
    And I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --delete"
   Then the exit code is 0
    And stdout does not contain "[DELETED]"
    And file "//{context.dnf.tempdir}/reposync/x86_64/wget-1.0-1.fc29.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/reposync/src/wget-1.0-1.fc29.src.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.3-1.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.2-1.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-thirdparty-updates/src/SuperRipper-1.3-1.src.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-thirdparty-updates/src/SuperRipper-1.2-1.src.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-thirdparty-updates/src/CQRlib-extension-1.6-2.src.rpm" exists


Scenario: Reposync preserves remote timestamps of packages
  Given I use repository "reposync" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --remote-time"
   Then the exit code is 0
    And stderr contains " wget-0:1\.0-1\.fc29\.x86_64"
    And stderr contains " wget-0:1\.0-1\.fc29\.src"
    And the files "{context.dnf.tempdir}/reposync/x86_64/wget-1.0-1.fc29.x86_64.rpm" and "{context.dnf.fixturesdir}/repos/reposync/x86_64/wget-1.0-1.fc29.x86_64.rpm" do not differ
    And timestamps of the files "{context.dnf.tempdir}/reposync/x86_64/wget-1.0-1.fc29.x86_64.rpm" and "{context.dnf.fixturesdir}/repos/reposync/x86_64/wget-1.0-1.fc29.x86_64.rpm" do not differ


Scenario: Reposync preserves remote timestamps of metadata files
  Given I use repository "reposync" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --download-metadata --remote-time"
   Then the exit code is 0
    And the files "{context.dnf.tempdir}/reposync/repodata/primary.xml.zst" and "{context.dnf.fixturesdir}/repos/reposync/repodata/primary.xml.zst" do not differ
    And timestamps of the files "{context.dnf.tempdir}/reposync/repodata/primary.xml.zst" and "{context.dnf.fixturesdir}/repos/reposync/repodata/primary.xml.zst" do not differ


@bz1686602
Scenario: Reposync --urls switch
  Given I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --urls"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout matches line by line
    """
    http://localhost:[0-9]+/src/CQRlib-extension-1\.6-2\.src\.rpm
    http://localhost:[0-9]+/src/SuperRipper-1\.2-1\.src\.rpm
    http://localhost:[0-9]+/src/SuperRipper-1\.3-1\.src\.rpm
    http://localhost:[0-9]+/x86_64/CQRlib-extension-1\.6-2\.x86_64\.rpm
    http://localhost:[0-9]+/x86_64/SuperRipper-1\.2-1\.x86_64\.rpm
    http://localhost:[0-9]+/x86_64/SuperRipper-1\.3-1\.x86_64\.rpm
    """


@bz1686602
Scenario: Reposync --urls and --download-metadata switches
  Given I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --urls --download-metadata"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout matches line by line
    """
    http://localhost:[0-9]+/repodata/primary.xml.zst
    http://localhost:[0-9]+/repodata/filelists.xml.zst
    http://localhost:[0-9]+/repodata/other.xml.zst
    http://localhost:[0-9]+/repodata/comps.xml.zst
    http://localhost:[0-9]+/src/CQRlib-extension-1\.6-2\.src\.rpm
    http://localhost:[0-9]+/src/SuperRipper-1\.2-1\.src\.rpm
    http://localhost:[0-9]+/src/SuperRipper-1\.3-1\.src\.rpm
    http://localhost:[0-9]+/x86_64/CQRlib-extension-1\.6-2\.x86_64\.rpm
    http://localhost:[0-9]+/x86_64/SuperRipper-1\.2-1\.x86_64\.rpm
    http://localhost:[0-9]+/x86_64/SuperRipper-1\.3-1\.x86_64\.rpm
    """


@xfail
# --newest-only is not yet fully supported for modules
# https://github.com/rpm-software-management/dnf5/issues/1902
@bz1775434
Scenario: Reposync --newest-only downloads packages from all streams and latest context versions of modular repository and latest non-modular rpms
  Given I use repository "dnf-ci-multicontext-hybrid-multiversion-modular" as http
   When I execute dnf with args "reposync --newest-only --download-path={context.dnf.tempdir}"
   Then the exit code is 0
    And file "//{context.dnf.tempdir}/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.4.1-2.module_2011+41787af1.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.4.1-2.module_3012+41787ba4.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.3.1-1.module_2011+41787af0.x86_64.rpm" does not exist
    And file "//{context.dnf.tempdir}/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.3.1-1.module_3012+41787ba3.x86_64.rpm" does not exist
    And file "//{context.dnf.tempdir}/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.12.1-1.fc29.x86_64.rpm" does not exist
    And file "//{context.dnf.tempdir}/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.12.2-3.fc29.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/postgresql-9.6.8-1.module_1710+b535a823.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/postgresql-9.8.1-1.module_9790+c535b823.x86_64.rpm" exists


@xfail
# --newest-only is not yet fully supported for modules
# https://github.com/rpm-software-management/dnf5/issues/1902
@bz1833074
Scenario: Reposync --newest-only downloads latest modular packages versions even if they are not part of the latest context version
  Given I use repository "reposync-newest-modular"
   When I execute dnf with args "reposync --newest-only --download-path={context.dnf.tempdir}"
   Then the exit code is 0
   When I execute "ls {context.dnf.tempdir}/reposync-newest-modular/x86_64/"
   # labirinto-0.9-1 is the highest non-modular NEVRA
   # labirinto-1.0-2 is part of the latest stream version
   # labirinto-1.0-9 is the highest modular NEVRA
   Then stdout is
        """
        labirinto-0.9-1.x86_64.rpm
        labirinto-1.0-2.module.x86_64.rpm
        labirinto-1.0-9.module.x86_64.rpm
        """


@bz1795965
Scenario: Reposync accepts --norepopath to synchronize single repository
  Given I use repository "reposync" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --norepopath"
   Then the exit code is 0
    And stderr contains " wget-0:1\.0-1\.fc29\.x86_64 .*"
    And stderr contains " wget-0:1\.0-1\.fc29\.src .*"
    And the files "{context.dnf.tempdir}/x86_64/wget-1.0-1.fc29.x86_64.rpm" and "{context.dnf.fixturesdir}/repos/reposync/x86_64/wget-1.0-1.fc29.x86_64.rpm" do not differ
    And the files "{context.dnf.tempdir}/src/wget-1.0-1.fc29.src.rpm" and "{context.dnf.fixturesdir}/repos/reposync/src/wget-1.0-1.fc29.src.rpm" do not differ


@bz1795965
Scenario: Reposync --norepopath cannot be used with multiple repositories
  Given I use repository "reposync" as http
    And I use repository "dnf-ci-thirdparty-updates" as http
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --norepopath"
   Then the exit code is 2
    And stderr is
    """
    Can't use --norepopath with multiple repositories enabled. Add "--help" for more information about the arguments.
    """


@bz1856818
Scenario: Reposync --gpgcheck removes unsigned packages and packages signed by not-installed keys
  Given I use repository "reposync-gpg" with configuration
        | key      | value      |
        | gpgcheck | 1          |
        | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/reposync-gpg/reposync-gpg-public |
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --gpgcheck"
   Then the exit code is 1
    And stderr contains lines matching
    """
    Removing '.*/reposync-gpg/src/dedalo-signed-1\.0-1\.fc29\.src\.rpm' with failing OpenPGP check: No corresponding key was found\.
    Removing '.*/reposync-gpg/x86_64/dedalo-signed-1\.0-1\.fc29\.x86_64\.rpm' with failing OpenPGP check: No corresponding key was found\.
    Removing '.*/reposync-gpg/src/dedalo-unsigned-1\.0-1\.fc29\.src\.rpm' with failing OpenPGP check: The package is not signed\.
    Removing '.*/reposync-gpg/x86_64/dedalo-unsigned-1\.0-1\.fc29\.x86_64\.rpm' with failing OpenPGP check: The package is not signed\.
    OpenPGP signature check failed
    """
    And file "//{context.dnf.tempdir}/reposync-gpg/x86_64/dedalo-unsigned-1.0-1.fc29.x86_64.rpm" does not exist
    And file "//{context.dnf.tempdir}/reposync-gpg/x86_64/dedalo-signed-1.0-1.fc29.x86_64.rpm" does not exist


@bz1856818
Scenario: Reposync --gpgcheck removes unsigned packages
  Given I use repository "reposync-gpg" with configuration
        | key      | value      |
        | gpgcheck | 1          |
        | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/reposync-gpg/reposync-gpg-public |
    # install package to ensure the key is imported to rpmdb
    And I successfully execute dnf with args "install dedalo-signed"
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir} --gpgcheck"
   Then the exit code is 1
    And stderr contains lines matching
    """
    Removing '.*/reposync-gpg/src/dedalo-unsigned-1\.0-1\.fc29\.src\.rpm' with failing OpenPGP check: The package is not signed\.
    Removing '.*/reposync-gpg/x86_64/dedalo-unsigned-1\.0-1\.fc29\.x86_64\.rpm' with failing OpenPGP check: The package is not signed\.
    OpenPGP signature check failed
    """
    And file "//{context.dnf.tempdir}/reposync-gpg/x86_64/dedalo-unsigned-1.0-1.fc29.x86_64.rpm" does not exist
    And file "//{context.dnf.tempdir}/reposync-gpg/x86_64/dedalo-signed-1.0-1.fc29.x86_64.rpm" exists


@xfail
# The exit code is currently 0, tracked in the issue
# https://github.com/rpm-software-management/dnf5/issues/1926
@bz2009894
Scenario: Reposync does not stop downloading packages on the first error
  Given I copy repository "simple-base" for modification
    And I use repository "simple-base"
    # remove one of reposynced packages to cause download error
    And I delete file "//{context.dnf.tempdir}/repos/simple-base/src/dedalo-signed-1.0-1.fc29.src.rpm"
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir}"
   Then the exit code is 1
    And stderr contains lines matching
    """
     dedalo-signed-0:1.0-1.fc29.src.*
    >>> Curl error.*
    >>> No more mirrors to try - All mirrors were already tried without success
    """
    # check that all other packages were downloaded
    And file "//{context.dnf.tempdir}/simple-base/src/labirinto-1.0-1.fc29.src.rpm" exists
    And file "//{context.dnf.tempdir}/simple-base/src/vagare-1.0-1.fc29.src.rpm" exists
    And file "//{context.dnf.tempdir}/simple-base/x86_64/dedalo-signed-1.0-1.fc29.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/simple-base/x86_64/labirinto-1.0-1.fc29.x86_64.rpm" exists
    And file "//{context.dnf.tempdir}/simple-base/x86_64/vagare-1.0-1.fc29.x86_64.rpm" exists


# https://issues.redhat.com/browse/RHEL-64320
Scenario: Reposync doesn't download duplicit nevra multiple times
  # create a repository containing a duplicated NEVRA
  Given I copy repository "simple-base" for modification
    And I execute "createrepo_c -n x86_64/labirinto-1.0-1.fc29.x86_64.rpm -n x86_64/labirinto-1.0-1.fc29.x86_64.rpm --duplicated-nevra=keep {context.dnf.tempdir}/repos/simple-base/"
    And I use repository "simple-base"
   When I execute dnf with args "reposync --download-path={context.dnf.tempdir}"
   Then the exit code is 0
    # check that the package have been downloaded
    And file "//{context.dnf.tempdir}/simple-base/x86_64/labirinto-1.0-1.fc29.x86_64.rpm" exists
    # check that the package was being downloaded only once
    And stderr contains "labirinto-0:1\.0-1\.fc29\.x86_64"
    # By default re.search() (used by "stdout does not contain") does not match
    # across multiple lines. To bypass this limitation and check that the package
    # name is not present on multiple lines, use "(.|\n)*" pattern instead of ".*".
    And stderr does not contain "labirinto-0:1\.0-1\.fc29\.x86_64(.|\n)*labirinto-1\.0-1\.fc29\.x86_64"


# https://redhat.atlassian.net/browse/RHEL-119475
Scenario: --min-buildtime option
  Given I copy repository "simple-base" for modification
    # rebuild all packages in simple-base repo with buildtime ~ 2005-3-16 19:06
    And I execute "SOURCE_DATE_EPOCH=1111000000 rpmbuild -rb --define "_rpmdir ." --define "use_source_date_epoch_as_buildtime 1" /{context.dnf.repos[simple-base].path}/src/*.src.rpm" in "{context.dnf.repos[simple-base].path}"
    # rebuild labirinto package with newer buildtime ~ 2011-3-13 07:06
    And I execute "SOURCE_DATE_EPOCH=1300000000 rpmbuild -rb --define "_rpmdir ." --define "use_source_date_epoch_as_buildtime 1" /{context.dnf.repos[simple-base].path}/src/labirinto*.src.rpm" in "{context.dnf.repos[simple-base].path}"
    # remove source rpms not to interfere with list outputs
    And I execute "rm -rf src" in "{context.dnf.repos[simple-base].path}"
    And I generate repodata for repository "simple-base"
    And I use repository "simple-base"
   When I execute dnf with args "reposync --urls --min-buildtime 2008-1-1"
   Then the exit code is 0
    And stdout matches line by line
    """
    .*/repos/simple-base/x86_64/labirinto-1\.0-1\.fc29\.x86_64\.rpm
    """
    And stderr is
    """
    <REPOSYNC>
    """
