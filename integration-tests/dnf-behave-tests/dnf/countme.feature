Feature: Better user counting

    @destructive
    @bz1777255
    @bz1676891
    Scenario Outline: User-Agent header is sent
        Given I am running a system identified as the "<system>"
          And I use repository "dnf-ci-fedora" as http
          And I start capturing outbound HTTP requests
         When I execute dnf with args "makecache --refresh"
         Then every HTTP GET request should match
            | header     | value   |
            | User-Agent | <agent> |

    Examples:
        | system                        | agent                                                         |
        | Fedora 29; server             | libdnf (Fedora 29; server; Linux.x86_64)                      |
        | Fedora 30; workstation        | libdnf (Fedora 30; workstation; Linux.x86_64)                 |
        | Fedora 31                     | libdnf (Fedora 31; generic; Linux.x86_64)                     |
        | Red Hat Enterprise Linux 8.1  | libdnf (Red Hat Enterprise Linux 8.1; generic; Linux.x86_64)  |
        | CentOS Linux 8.1              | libdnf (CentOS Linux 8.1; generic; Linux.x86_64)              |

    @destructive
    Scenario: No os-release file installed
        Given I remove the os-release file
          And I use repository "dnf-ci-fedora" as http
          And I start capturing outbound HTTP requests
         When I execute dnf with args "makecache --refresh"
         Then the exit code is 0
          And every HTTP GET request should match
            | header     | value  |
            | User-Agent | libdnf |

    Scenario: Custom User-Agent value
        Given I use repository "dnf-ci-fedora" as http
          And I set config option "user_agent" to "'Agent 007'"
          And I start capturing outbound HTTP requests
         When I execute dnf with args "makecache --refresh"
         Then every HTTP GET request should match
            | header     | value     |
            | User-Agent | Agent 007 |

    @destructive
    Scenario Outline: Countme flag is sent once per calendar week
        Given the machine-id file is <machine-id> as of <epoch>
          And I set config option "countme" to "1"
          And I set releasever to "39"
          And I copy repository "dnf-ci-fedora" for modification
          And I use repository "dnf-ci-fedora" as http
          And I set up metalink for repository "dnf-ci-fedora"
          And I start capturing outbound HTTP requests

         # First calendar week
         # Note: One in the first 4 requests is randomly chosen to include the
         # flag (see COUNTME_BUDGET=4 in libdnf/repo/Repo.cpp for details)
         When today is <date>
         When I execute dnf with args "makecache --refresh" 4 times
         Then exactly one HTTP GET request should match
            | path                              |
            | */metalink.xml*&countme=<age#1>   |

         # Same calendar week (should not be sent)
         When today is <date> + 3 days
          And I forget any HTTP requests captured so far
          And I execute dnf with args "makecache --refresh" 4 times
         Then no HTTP GET request should match
            | path                              |
            | */metalink.xml*&countme=*         |

         # Next calendar week
         When today is <date> + 8 days
          And I forget any HTTP requests captured so far
          And I execute dnf with args "makecache --refresh" 4 times
         Then exactly one HTTP GET request should match
            | path                              |
            | */metalink.xml*&countme=<age#2>   |

         # Next calendar week
         When today is <date> + 15 days
          And I forget any HTTP requests captured so far
          And I execute dnf with args "makecache --refresh" 4 times
         Then exactly one HTTP GET request should match
            | path                              |
            | */metalink.xml*&countme=<age#3>   |

         # Next calendar month
         When today is <date> + 40 days
          And I forget any HTTP requests captured so far
          And I execute dnf with args "makecache --refresh" 4 times
         Then exactly one HTTP GET request should match
            | path                              |
            | */metalink.xml*&countme=<age#4>   |

         # 6 calendar months later
         When today is <date> + 182 days
          And I forget any HTTP requests captured so far
          And I execute dnf with args "makecache --refresh" 4 times
         Then exactly one HTTP GET request should match
            | path                              |
            | */metalink.xml*&countme=<age#5>   |

         # Even later, after a system upgrade
         When today is <date> + 365 days
          And I set releasever to "40"
          And I forget any HTTP requests captured so far
          And I execute dnf with args "makecache --refresh" 4 times
         Then exactly one HTTP GET request should match
            | path                              |
            | */metalink.xml*&countme=<age#6>   |

    Examples:
        | machine-id    | epoch         | date          | age#1 | age#2 | age#3 | age#4 | age#5 | age#6 |
        # Absolute age counting (since "epoch")
        | initialized   | Aug 06, 2019  | Aug 07, 2019  | 1     | 1     | 2     | 3     | 4     | 4     |
        | initialized   | Aug 06, 2019  | Aug 20, 2019  | 2     | 2     | 2     | 3     | 4     | 4     |
        | initialized   | Aug 06, 2019  | Sep 12, 2019  | 3     | 3     | 3     | 3     | 4     | 4     |
        | initialized   | Aug 06, 2019  | Jun 18, 2020  | 4     | 4     | 4     | 4     | 4     | 4     |
        # Relative age counting (since "date")
        | uninitialized | Aug 06, 2019  | Jun 18, 2020  | 1     | 1     | 2     | 3     | 4     | 1     |
        | empty         | ---           | Jun 18, 2020  | 1     | 1     | 2     | 3     | 4     | 1     |
        | absent        | ---           | Jun 18, 2020  | 1     | 1     | 2     | 3     | 4     | 1     |

    @destructive
    Scenario: Countme flag is not sent repeatedly on retries
        Given the machine-id file is initialized as of today
          And I set config option "countme" to "1"
          And I copy repository "dnf-ci-fedora" for modification
          And I use repository "dnf-ci-fedora" as http
          And I set up metalink for repository "dnf-ci-fedora"
          # This triggers the retry mechanism in librepo, 4 retries by default
          And the server starts responding with HTTP status code 503
          And I start capturing outbound HTTP requests
         When I execute dnf with args "makecache --refresh" 4 times
         # 48 = 4 * makecache --refresh = 4 * (3 metalink attempts * 4 low-level retries)
         # See librepo commits 15adfb31 and 12d0b4ad for details
         Then exactly 48 HTTP GET requests should match
            | path            |
            | */metalink.xml* |
          And exactly one HTTP GET request should match
            | path                      |
            | */metalink.xml*&countme=1 |

    @destructive
    Scenario: Countme feature is disabled
        Given the machine-id file is initialized as of today
          And I set config option "countme" to "0"
          And I copy repository "dnf-ci-fedora" for modification
          And I use repository "dnf-ci-fedora" as http
          And I set up metalink for repository "dnf-ci-fedora"
          And I start capturing outbound HTTP requests
         When I execute dnf with args "makecache --refresh" 4 times
         Then no HTTP GET request should match
            | path                      |
            | */metalink.xml*&countme=* |
