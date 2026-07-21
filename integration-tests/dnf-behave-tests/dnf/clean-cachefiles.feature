@dnf5daemon
Feature: Testing that dnf clean command removes files from the cache


Background: Fill the cache
  Given I use repository "simple-base" as http
   When I execute dnf with args "--setopt=keepcache=true install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | labirinto-0:1.0-1.fc29.x86_64             |
   # ensure that metadata are present
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \.
   \./simple-base-[0-9a-f]{16}
   \./simple-base-[0-9a-f]{16}/packages
   \./simple-base-[0-9a-f]{16}/packages/labirinto-1\.0-1\.fc29\.x86_64\.rpm
   \./simple-base-[0-9a-f]{16}/repodata
   \./simple-base-[0-9a-f]{16}/repodata/primary\.xml\.zst
   \./simple-base-[0-9a-f]{16}/repodata/repomd\.xml
   \./simple-base-[0-9a-f]{16}/solv
   \./simple-base-[0-9a-f]{16}/solv/simple-base\.solv
   """


Scenario: Cleanup of the whole cache (dnf clean all)
   When I execute dnf with args "clean all"
   Then the exit code is 0
    And stdout matches line by line
        """
        (Removed 4 files, 4 directories \(total of [0-9]* KiB\). 0 errors occurred.|Cache successfully cleaned.)
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \.
   """


Scenario: Cleanup of the whole cache, cache contains link pointing outside, content outside the cache must not be cleaned!
  # Creating a directory and files outside the cache
  Given I create directory "/tmp/user_dir"
    And I create file "/tmp/user_dir/user_file1" with
    """
    User data 1
    """
    And I create file "/tmp/user_file2" with
    """
    User data 2
    """
    # Poison the cache, add symbolic links pointing to the directory and file outside the cache
    And I execute "ln -s {context.dnf.installroot}/tmp/user_dir `ls -d simple-base-*`/packages/link_outside1" in "{context.dnf.installroot}/var/cache/dnf"
    And I execute "ln -s {context.dnf.installroot}/tmp/user_file2 `ls -d simple-base-*`/packages/link_outside2" in "{context.dnf.installroot}/var/cache/dnf"
   When I execute dnf with args "clean all"
   Then the exit code is 0
    And stdout matches line by line
        """
        (Removed 6 files, 4 directories \(total of [0-9]* KiB\). 0 errors occurred.|Cache successfully cleaned.)
        """
    And directory "/var/cache/dnf" is empty
    And file "/tmp/user_dir/user_file1" exists
    And file "/tmp/user_file2" exists


Scenario: Cached metadata cleanup (dnf clean metadata)
   When I execute dnf with args "clean metadata"
   Then the exit code is 0
    And stdout matches line by line
        """
        (Removed 3 files, 2 directories \(total of [0-9]* KiB\). 0 errors occurred.|Cache successfully cleaned.)
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \.
   \./simple-base-[0-9a-f]{16}
   \./simple-base-[0-9a-f]{16}/packages
   \./simple-base-[0-9a-f]{16}/packages/labirinto-1\.0-1\.fc29\.x86_64\.rpm
   """


Scenario: Cached packages cleanup (dnf clean packages)
   When I execute dnf with args "clean packages"
   Then the exit code is 0
    And stdout matches line by line
        """
        (Removed 1 files, 1 directories \(total of [0-9]* KiB\). 0 errors occurred.|Cache successfully cleaned.)
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \.
   \./simple-base-[0-9a-f]{16}
   \./simple-base-[0-9a-f]{16}/repodata
   \./simple-base-[0-9a-f]{16}/repodata/primary\.xml\.zst
   \./simple-base-[0-9a-f]{16}/repodata/repomd\.xml
   \./simple-base-[0-9a-f]{16}/solv
   \./simple-base-[0-9a-f]{16}/solv/simple-base\.solv
   """


Scenario: Database cached cleanup (dnf clean dbcache)
   When I execute dnf with args "clean dbcache"
   Then the exit code is 0
    And stdout matches line by line
        """
        (Removed 1 files, 1 directories \(total of [0-9]* KiB\). 0 errors occurred.|Cache successfully cleaned.)
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \.
   \./simple-base-[0-9a-f]{16}
   \./simple-base-[0-9a-f]{16}/packages
   \./simple-base-[0-9a-f]{16}/packages/labirinto-1\.0-1\.fc29\.x86_64\.rpm
   \./simple-base-[0-9a-f]{16}/repodata
   \./simple-base-[0-9a-f]{16}/repodata/primary\.xml\.zst
   \./simple-base-[0-9a-f]{16}/repodata/repomd\.xml
   """
