Feature: api: package


Scenario: Filter one package and get various info about it
Given I use repository "simple-base"
 When I execute python libdnf5 api script with setup
      """
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_nevra(["vagare-1.0-1.fc29.x86_64"])
      for pkg in query:
          print(pkg.get_name())
          print(pkg.get_epoch())
          print(pkg.get_version())
          print(pkg.get_release())
          print(pkg.get_evr())
          print(pkg.get_full_nevra())
          print(pkg.get_na())
          print(pkg.get_license())
          print(pkg.get_sourcerpm())
          print(pkg.get_summary())
          provs = [prov.get_name() for prov in pkg.get_provides()]
          for prov in sorted(provs):
              print(prov)
          reqs = pkg.get_requires()
          for req in reqs:
              print(req.get_name())
          print(pkg.is_installed())
      """
 Then the exit code is 0
  And stdout is
      """
      vagare
      0
      1.0
      1.fc29
      1.0-1.fc29
      vagare-0:1.0-1.fc29.x86_64
      vagare.x86_64
      GPLv3+
      vagare-1.0-1.fc29.src.rpm
      Made up package
      vagare
      vagare(x86-64)
      labirinto
      False
      """


Scenario: Try to get info about a non-existent package
 When I execute python libdnf5 api script with setup
      """
      query = libdnf5.rpm.PackageQuery(base)
      query.filter_name(["nosuchpackage"])
      for pkg in query:
          print(pkg.get_name())
      """
 Then the exit code is 0
  And stdout is empty
