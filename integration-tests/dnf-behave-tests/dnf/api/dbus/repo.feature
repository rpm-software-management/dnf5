@not.with_mode=dnf5
@dnf5daemon
Feature: D-Bus api: repo


Scenario: basic repo info
Given I use repository "simple-base"
 When I execute python libdnf5 dbus api script with repo interface
    """
    options = {{
        "patterns" : ["simple-base"],
        "repo_attrs" : ["id", "enabled"],
    }}
    print(dbus_to_python(iface_repo.list(options)))
    """
 Then stdout is
    """
    [{{'enabled': True, 'id': 'simple-base'}}]
    """


Scenario: repo info with repo with duplicate packages
Given I create directory "/duplicates"
  And I create directory "/duplicates/a"
  And I create directory "/duplicates/b"
  And I copy file "{context.dnf.fixturesdir}/repos/simple-base/x86_64/labirinto-1.0-1.fc29.x86_64.rpm" to "/duplicates/a/labirinto-1.0-1.fc29.x86_64.rpm"
  And I copy file "{context.dnf.fixturesdir}/repos/simple-base/x86_64/labirinto-1.0-1.fc29.x86_64.rpm" to "/duplicates/b/labirinto-1.0-1.fc29.x86_64.rpm"
  And I execute "createrepo_c {context.dnf.installroot}/duplicates"
  And I configure a new repository "testrepo" with
      | key        | value                                |
      | baseurl    | {context.dnf.installroot}/duplicates |
 When I execute python libdnf5 dbus api script with repo interface
    """
    options = {{
        "patterns" : ["testrepo"],
        "repo_attrs" : ["id", "enabled", "pkgs", "unique_nevras"],
    }}
    print(dbus_to_python(iface_repo.list(options)))
    """
 Then stdout is
    """
    [{{'enabled': True, 'id': 'testrepo', 'pkgs': 2, 'unique_nevras': 1}}]
    """
