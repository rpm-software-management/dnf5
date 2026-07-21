Feature: Tests for cache


@bz2027445
Scenario: Regenerate solvfile cache when solvfile version doesn't match
  Given I use repository "simple-base"
    And I execute dnf with args "makecache"
   When I invalidate solvfile version of "{context.dnf.installroot}/var/cache/dnf/simple-base-*/solv/simple-base.solv"
    And I execute dnf with args "repoquery"
   Then file "/var/log/dnf5.log" contains lines
        """
        .*WARNING Libsolv solvfile version doesn't match.*
        """
