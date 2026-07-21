@destructive
@no_installroot
Feature: test dnf5 as unprivileged user


Scenario: unprivileged user can create a cache even without system state
  Given I use repository "simple-base"
    # Give user permissions to access cache
    And I create directory "/{context.dnf.installroot}/var/cache/dnf"
    And I successfully execute "chmod 777 {context.dnf.installroot}/var/cache/dnf"
    And I delete directory "/usr/lib/sysimage/libdnf5"
   When I execute dnf with args "makecache" as an unprivileged user
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Metadata cache created.
        """
