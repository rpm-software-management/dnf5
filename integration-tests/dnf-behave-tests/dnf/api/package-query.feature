@not.with_dnf=4
Feature: api: query packages


Scenario: Construct queries and filter packages by name
Given I use repository "simple-base"
 When I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_CONTAINS, QueryCmp_GLOB
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_name(["labirinto"])
      for pkg in query:
          print(pkg.get_nevra())
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_name(["*da?*"], QueryCmp_GLOB)
      for pkg in query:
          print(pkg.get_nevra())
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_name(["gar"], QueryCmp_CONTAINS)
      for pkg in query:
          print(pkg.get_nevra())
      """
 Then the exit code is 0
  And stdout is
      """
      labirinto-1.0-1.fc29.src
      labirinto-1.0-1.fc29.x86_64
      dedalo-1.0-1.fc29.src
      dedalo-1.0-1.fc29.x86_64
      vagare-1.0-1.fc29.src
      vagare-1.0-1.fc29.x86_64
      """


Scenario: Construct query and filter fails due to bad argument type
Given I use repository "simple-base"
 When I execute python libdnf5 api script with setup
      """
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_name(99)
      for pkg in query:
          print(pkg.get_nevra())
      """
 Then the exit code is 1
  And stdout is empty
  And stderr contains "TypeError: Wrong number or type of arguments for overloaded function 'PackageQuery_filter_name'."


Scenario: Construct queries and filter packages by arch
Given I use repository "dnf-ci-fedora"
 When I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_NEQ, QueryCmp_NOT_GLOB
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["noarch"])
      for pkg in query:
          print(pkg.get_na())
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["x86*","*arch","?rc"], QueryCmp_NOT_GLOB)
      for pkg in query:
          print(pkg.get_na())
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["x86_64","noarch","src"], QueryCmp_NEQ)
      for pkg in query:
          print(pkg.get_nevra())
      """
 Then the exit code is 0
  And stdout is
      """
      abcde.noarch
      basesystem.noarch
      fedora-release.noarch
      nodejs-docs.noarch
      setup.noarch
      lz4.i686
      lz4-debuginfo.i686
      lz4-devel.i686
      lz4-static.i686
      lz4-1.7.5-2.fc26.i686
      lz4-debuginfo-1.7.5-2.fc26.i686
      lz4-devel-1.7.5-2.fc26.i686
      lz4-static-1.7.5-2.fc26.i686
      """


Scenario: Construct queries and filter packages by conflicts
Given I use repository "dnf-ci-rich"
 When I execute python libdnf5 api script with setup
      """
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_name(["milk"])
      query.filter_arch(["x86_64"])
      # milk conflicts with water; we get a conflict-list of milk first
      # (it's a ReldepList containing water) and then we filter all packages
      # that conflict with water (it's only milk)
      query2 = libdnf5.rpm.PackageQuery(base)
      for pkg in query:
          query2.filter_conflicts(pkg.get_conflicts())
      for pkg in query2:
          print(pkg.get_nevra())
      # now find water's conflicts differently: via PackageSet with Package
      # objects
      query3 = libdnf5.rpm.PackageQuery(base)
      query3.filter_name(["water"])
      query3.filter_arch(["x86_64"])
      query4 = libdnf5.rpm.PackageQuery(base)
      query4.filter_conflicts(query3)
      for pkg in query4:
          print(pkg.get_name())
      """
 Then the exit code is 0
  And stdout is
      """
      milk-1.0-1.x86_64
      milk
      """


Scenario: Construct queries and filter packages by description
Given I use repository "simple-base"
 When I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_GLOB
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["x86_64"])
      query.filter_description(["*script*"], QueryCmp_GLOB)
      filtered = [pkg.get_name() for pkg in query]
      for pkg in sorted(filtered):
          print(pkg)
      query2 = libdnf5.rpm.PackageQuery(base)
      query2.filter_arch(["x86_64"])
      query2.filter_description(["vagare description"])
      for pkg in query2:
          print(pkg.get_nevra())
      """
 Then the exit code is 0
  And stdout is
      """
      dedalo
      labirinto
      vagare
      vagare-1.0-1.fc29.x86_64
      """


Scenario: Construct queries and filter packages by files
Given I use repository "dnf-ci-fedora"
  And I use repository "plugins-callbacks"
 When I execute dnf with args "install glibc watermelon"
  And I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_IEXACT, QueryCmp_GLOB
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["x86_64"])
      query.filter_file(["*dnf-plugin*"], QueryCmp_GLOB)
      # NOTE: filtered files are not in primary, only in filelists, so
      # no available pkgs (3 versions of watermelon in the repo) are found.
      # config.get_optional_metadata_types_option().set(libdnf5.conf.METADATA_TYPE_FILELISTS)
      # can be used before the repo is loaded to get also filelists
      for pkg in query:
          print(pkg.get_nevra())
      query2 = libdnf5.rpm.PackageQuery(base)
      query2.filter_arch(["x86_64"])
      query2.filter_file(["/ETC/ld.so.conf"], QueryCmp_IEXACT)
      query2.filter_installed()
      # NOTE: without filter_installed, there would be glibc twice;
      # the file is in primary, so available pkg in the repo is found, too
      for pkg in query2:
          print(pkg.get_nevra())
      """
 Then the exit code is 0
  And stdout is
      """
      watermelon-3.0-1.fc29.x86_64
      glibc-2.28-9.fc29.x86_64
      """


Scenario: Construct queries and filter packages by epoch
Given I use repository "repoquery-main"
 When I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_NEQ
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["x86_64"])
      query.filter_epoch(["2"])
      for pkg in query:
          print(pkg.get_nevra())
      query2 = libdnf5.rpm.PackageQuery(base)
      query2.filter_epoch(["1","0"], QueryCmp_NEQ)
      for pkg in query2:
          print(pkg.get_nevra())
      """
 Then the exit code is 0
  And stdout is
      """
      top-a-2:2.0-2.x86_64
      top-a-2:2.0-2.src
      top-a-2:2.0-2.x86_64
      """


Scenario: Construct queries and filter packages by repo_id
Given I use repository "simple-base"
  And I use repository "dnf-ci-fedora"
 When I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_GLOB
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["x86_64"])
      query.filter_repo_id(["simple-base"])
      pkgs = [pkg.get_name() for pkg in query]
      for pkg in sorted(pkgs):
          print(pkg)
      query2 = libdnf5.rpm.PackageQuery(base)
      query2.filter_repo_id(["*base"], QueryCmp_GLOB)
      pkgs2 = [pkg.get_nevra() for pkg in query2]
      for pkg in sorted(pkgs2):
          print(pkg)
      """
 Then the exit code is 0
  And stdout is
      """
      dedalo
      labirinto
      vagare
      dedalo-1.0-1.fc29.src
      dedalo-1.0-1.fc29.x86_64
      labirinto-1.0-1.fc29.src
      labirinto-1.0-1.fc29.x86_64
      vagare-1.0-1.fc29.src
      vagare-1.0-1.fc29.x86_64
      """


Scenario: Construct queries and filter packages by sourcerpm
Given I use repository "dnf-ci-fedora-updates"
 When I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_GLOB
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["x86_64"])
      query.filter_sourcerpm(["flac-1.3.3-2.fc29.src.rpm"])
      pkgs = [pkg.get_nevra() for pkg in query]
      for pkg in sorted(pkgs):
          print(pkg)
      query2 = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["x86_64"])
      query2.filter_sourcerpm(["flac*"], QueryCmp_GLOB)
      pkgs2 = [pkg.get_nevra() for pkg in query2]
      for pkg in sorted(pkgs2):
          print(pkg)
      """
 Then the exit code is 0
  And stdout is
      """
      flac-1.3.3-2.fc29.x86_64
      flac-libs-1.3.3-2.fc29.x86_64
      flac-1.3.3-1.fc29.x86_64
      flac-1.3.3-2.fc29.x86_64
      flac-1.3.3-3.fc29.x86_64
      flac-libs-1.3.3-1.fc29.x86_64
      flac-libs-1.3.3-2.fc29.x86_64
      flac-libs-1.3.3-3.fc29.x86_64
      """


Scenario: Construct queries and filter upgradable/upgrades
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install flac dwm wget"
  And I use repository "dnf-ci-fedora-updates"
  And I execute python libdnf5 api script with setup
      """
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_arch(["x86_64"])
      query.filter_upgradable()
      pkgs = [pkg.get_nevra() for pkg in query]
      for pkg in sorted(pkgs):
          print(pkg)
      query2 = libdnf5.rpm.PackageQuery(base)
      query2.filter_upgrades()
      pkgs2 = [pkg.get_nevra() for pkg in query2]
      for pkg in sorted(pkgs2):
          print(pkg)
      """
 Then the exit code is 0
  And stdout is
      """
      flac-1.3.2-8.fc29.x86_64
      wget-1.19.5-5.fc29.x86_64
      flac-1.3.3-1.fc29.x86_64
      flac-1.3.3-2.fc29.x86_64
      flac-1.3.3-3.fc29.x86_64
      wget-1.19.6-5.fc29.x86_64
      """
