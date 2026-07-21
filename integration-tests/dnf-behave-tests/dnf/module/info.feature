# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Module info


Background:
Given I use repository "dnf-ci-fedora"
Given I use repository "dnf-ci-fedora-modular"
 When I execute dnf with args "module enable nodejs:8"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 8         |           |
 When I execute dnf with args "module install nodejs/default"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                       |
      | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | install-group             | npm-1:8.11.4-1.module_2030+42747d40.x86_64    |
      | module-profile-install    | nodejs/default                                |
Given I use repository "dnf-ci-fedora-modular-updates"
 When I execute dnf with args "module enable postgresql:11"
 Then the exit code is 0
 When I execute dnf with args "module install postgresql/client"
 Then the exit code is 0
  And modules state is following
      | Module        | State     | Stream    | Profiles      |
      | postgresql    | enabled   | 11        | client        |
  And Transaction contains
      | Action                    | Package                                          |
      | install-group             | postgresql-0:11.1-2.module_2597+e45c4cc9.x86_64  |
      | module-profile-install    | postgresql/client                                |


Scenario: Get info for a module, only module name specified
 When I execute dnf with args "module info nodejs"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      Name             : nodejs
      Stream           : 10
      Version          : 20180920144631
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default [d], development, minimal
      Default profiles : default
      Repo             : dnf-ci-fedora-modular
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
      Artifacts        : nodejs-1:10.11.0-1.module_2200+adbac02b.src
                       : nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64
                       : nodejs-devel-1:10.11.0-1.module_2200+adbac02b.x86_64
                       : nodejs-docs-1:10.11.0-1.module_2200+adbac02b.noarch
                       : npm-1:10.11.0-1.module_2200+adbac02b.x86_64

      Name             : nodejs
      Stream           : 10
      Version          : 20190102201818
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default [d], development, minimal
      Default profiles : default
      Repo             : dnf-ci-fedora-modular-updates
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
      Artifacts        : http-parser-0:2.9.0-1.module_2672+97d6a5e9.src
                       : http-parser-0:2.9.0-1.module_2672+97d6a5e9.x86_64
                       : http-parser-devel-0:2.9.0-1.module_2672+97d6a5e9.x86_64
                       : libnghttp2-0:1.35.1-1.module_2672+97d6a5e9.x86_64
                       : libnghttp2-devel-0:1.35.1-1.module_2672+97d6a5e9.x86_64
                       : libuv-1:1.23.2-1.module_2302+4c6ccf2f.x86_64
                       : libuv-devel-1:1.23.2-1.module_2302+4c6ccf2f.x86_64
                       : libuv-static-1:1.23.2-1.module_2302+4c6ccf2f.x86_64
                       : nghttp2-0:1.35.1-1.module_2672+97d6a5e9.x86_64
                       : nodejs-1:10.14.1-1.module_2533+7361f245.src
                       : nodejs-1:10.14.1-1.module_2533+7361f245.x86_64
                       : nodejs-devel-1:10.14.1-1.module_2533+7361f245.x86_64
                       : nodejs-docs-1:10.14.1-1.module_2533+7361f245.noarch
                       : npm-1:10.14.1-1.module_2533+7361f245.x86_64

      Name             : nodejs
      Stream           : 11
      Version          : 20180920144611
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default, development, minimal
      Default profiles :
      Repo             : dnf-ci-fedora-modular
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
                       : postgresql:[9.6]
      Artifacts        : nodejs-1:11.0.0-1.module_2311+8d497411.src
                       : nodejs-1:11.0.0-1.module_2311+8d497411.x86_64
                       : nodejs-devel-1:11.0.0-1.module_2311+8d497411.x86_64
                       : nodejs-docs-1:11.0.0-1.module_2311+8d497411.noarch
                       : npm-1:11.0.0-1.module_2311+8d497411.x86_64

      Name             : nodejs
      Stream           : 11
      Version          : 20181102165620
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default, development, minimal
      Default profiles :
      Repo             : dnf-ci-fedora-modular-updates
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
                       : postgresql:[9.6]
      Artifacts        : libnghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                       : libnghttp2-devel-0:1.34.0-1.module_2365+652bf990.x86_64
                       : libuv-1:1.23.2-1.module_2365+652bf990.x86_64
                       : libuv-devel-1:1.23.2-1.module_2365+652bf990.x86_64
                       : libuv-static-1:1.23.2-1.module_2365+652bf990.x86_64
                       : nghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                       : nodejs-1:11.1.0-1.module_2379+8d497405.src
                       : nodejs-1:11.1.0-1.module_2379+8d497405.x86_64
                       : nodejs-devel-1:11.1.0-1.module_2379+8d497405.x86_64
                       : nodejs-docs-1:11.1.0-1.module_2379+8d497405.noarch
                       : npm-1:11.1.0-1.module_2379+8d497405.x86_64

      Name             : nodejs
      Stream           : 12
      Version          : 20181102165620
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default, development, minimal
      Default profiles :
      Repo             : dnf-ci-fedora-modular-updates
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
                       : postgresql:[]
      Artifacts        : libnghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                       : libnghttp2-devel-0:1.34.0-1.module_2365+652bf990.x86_64
                       : libuv-1:1.23.2-1.module_2365+652bf990.x86_64
                       : libuv-devel-1:1.23.2-1.module_2365+652bf990.x86_64
                       : libuv-static-1:1.23.2-1.module_2365+652bf990.x86_64
                       : nghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                       : nodejs-1:12.1.0-1.module_2379+8d497405.src
                       : nodejs-1:12.1.0-1.module_2379+8d497405.x86_64
                       : nodejs-devel-1:12.1.0-1.module_2379+8d497405.x86_64
                       : nodejs-docs-1:12.1.0-1.module_2379+8d497405.noarch
                       : npm-1:12.1.0-1.module_2379+8d497405.x86_64

      Name             : nodejs
      Stream           : 5
      Version          : 20150811143428
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default, development, minimal
      Default profiles :
      Repo             : dnf-ci-fedora-modular
      Summary          : Javascript runtime module with quite a long
                       : summary that contains an empty line.
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
      Artifacts        : nodejs-1:5.3.1-1.module_2011+41787af0.src
                       : nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
                       : nodejs-devel-1:5.3.1-1.module_2011+41787af0.x86_64
                       : nodejs-docs-1:5.3.1-1.module_2011+41787af0.noarch
                       : npm-1:5.3.1-1.module_2011+41787af0.x86_64

      Name             : nodejs
      Stream           : 8 [d][e][a]
      Version          : 20180801080000
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default [d] [i], development, minimal
      Default profiles : default
      Repo             : dnf-ci-fedora-modular
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
      Artifacts        : nodejs-1:8.11.4-1.module_2030+42747d40.src
                       : nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
                       : nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
                       : nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch
                       : npm-1:8.11.4-1.module_2030+42747d40.x86_64

      Name             : nodejs
      Stream           : 8 [d][e][a]
      Version          : 20181216123422
      Context          : 7f892346
      Architecture     : x86_64
      Profiles         : default [d] [i], development, minimal
      Default profiles : default
      Repo             : dnf-ci-fedora-modular-updates
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
      Artifacts        : nodejs-1:8.11.4-1.module_2030+42747d40.src
                       : nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
                       : nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
                       : nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch
                       : npm-1:8.14.0-1.module_2030+42747d41.x86_64

      Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
      """


Scenario: Get info for an enabled stream, module name and stream specified
 When I execute dnf with args "module info nodejs:11"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      Name             : nodejs
      Stream           : 11
      Version          : 20180920144611
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default, development, minimal
      Default profiles :
      Repo             : dnf-ci-fedora-modular
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
                       : postgresql:[9.6]
      Artifacts        : nodejs-1:11.0.0-1.module_2311+8d497411.src
                       : nodejs-1:11.0.0-1.module_2311+8d497411.x86_64
                       : nodejs-devel-1:11.0.0-1.module_2311+8d497411.x86_64
                       : nodejs-docs-1:11.0.0-1.module_2311+8d497411.noarch
                       : npm-1:11.0.0-1.module_2311+8d497411.x86_64

      Name             : nodejs
      Stream           : 11
      Version          : 20181102165620
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default, development, minimal
      Default profiles :
      Repo             : dnf-ci-fedora-modular-updates
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
                       : postgresql:[9.6]
      Artifacts        : libnghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                       : libnghttp2-devel-0:1.34.0-1.module_2365+652bf990.x86_64
                       : libuv-1:1.23.2-1.module_2365+652bf990.x86_64
                       : libuv-devel-1:1.23.2-1.module_2365+652bf990.x86_64
                       : libuv-static-1:1.23.2-1.module_2365+652bf990.x86_64
                       : nghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                       : nodejs-1:11.1.0-1.module_2379+8d497405.src
                       : nodejs-1:11.1.0-1.module_2379+8d497405.x86_64
                       : nodejs-devel-1:11.1.0-1.module_2379+8d497405.x86_64
                       : nodejs-docs-1:11.1.0-1.module_2379+8d497405.noarch
                       : npm-1:11.1.0-1.module_2379+8d497405.x86_64

      Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
      """


  @bz1540189
  Scenario: Get info for an installed profile, module name and profile specified
   When I execute dnf with args "module info nodejs/minimal"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Ignoring unnecessary profile: 'nodejs/minimal'
        Name             : nodejs
        Stream           : 10
        Version          : 20180920144631
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default [d], development, minimal
        Default profiles : default
        Repo             : dnf-ci-fedora-modular
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
        Artifacts        : nodejs-1:10.11.0-1.module_2200+adbac02b.src
                         : nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64
                         : nodejs-devel-1:10.11.0-1.module_2200+adbac02b.x86_64
                         : nodejs-docs-1:10.11.0-1.module_2200+adbac02b.noarch
                         : npm-1:10.11.0-1.module_2200+adbac02b.x86_64

        Name             : nodejs
        Stream           : 10
        Version          : 20190102201818
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default [d], development, minimal
        Default profiles : default
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
        Artifacts        : http-parser-0:2.9.0-1.module_2672+97d6a5e9.src
                         : http-parser-0:2.9.0-1.module_2672+97d6a5e9.x86_64
                         : http-parser-devel-0:2.9.0-1.module_2672+97d6a5e9.x86_64
                         : libnghttp2-0:1.35.1-1.module_2672+97d6a5e9.x86_64
                         : libnghttp2-devel-0:1.35.1-1.module_2672+97d6a5e9.x86_64
                         : libuv-1:1.23.2-1.module_2302+4c6ccf2f.x86_64
                         : libuv-devel-1:1.23.2-1.module_2302+4c6ccf2f.x86_64
                         : libuv-static-1:1.23.2-1.module_2302+4c6ccf2f.x86_64
                         : nghttp2-0:1.35.1-1.module_2672+97d6a5e9.x86_64
                         : nodejs-1:10.14.1-1.module_2533+7361f245.src
                         : nodejs-1:10.14.1-1.module_2533+7361f245.x86_64
                         : nodejs-devel-1:10.14.1-1.module_2533+7361f245.x86_64
                         : nodejs-docs-1:10.14.1-1.module_2533+7361f245.noarch
                         : npm-1:10.14.1-1.module_2533+7361f245.x86_64

        Name             : nodejs
        Stream           : 11
        Version          : 20180920144611
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default, development, minimal
        Default profiles :
        Repo             : dnf-ci-fedora-modular
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
                         : postgresql:[9.6]
        Artifacts        : nodejs-1:11.0.0-1.module_2311+8d497411.src
                         : nodejs-1:11.0.0-1.module_2311+8d497411.x86_64
                         : nodejs-devel-1:11.0.0-1.module_2311+8d497411.x86_64
                         : nodejs-docs-1:11.0.0-1.module_2311+8d497411.noarch
                         : npm-1:11.0.0-1.module_2311+8d497411.x86_64

        Name             : nodejs
        Stream           : 11
        Version          : 20181102165620
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default, development, minimal
        Default profiles :
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
                         : postgresql:[9.6]
        Artifacts        : libnghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                         : libnghttp2-devel-0:1.34.0-1.module_2365+652bf990.x86_64
                         : libuv-1:1.23.2-1.module_2365+652bf990.x86_64
                         : libuv-devel-1:1.23.2-1.module_2365+652bf990.x86_64
                         : libuv-static-1:1.23.2-1.module_2365+652bf990.x86_64
                         : nghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                         : nodejs-1:11.1.0-1.module_2379+8d497405.src
                         : nodejs-1:11.1.0-1.module_2379+8d497405.x86_64
                         : nodejs-devel-1:11.1.0-1.module_2379+8d497405.x86_64
                         : nodejs-docs-1:11.1.0-1.module_2379+8d497405.noarch
                         : npm-1:11.1.0-1.module_2379+8d497405.x86_64

        Name             : nodejs
        Stream           : 12
        Version          : 20181102165620
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default, development, minimal
        Default profiles :
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
                         : postgresql:[]
        Artifacts        : libnghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                         : libnghttp2-devel-0:1.34.0-1.module_2365+652bf990.x86_64
                         : libuv-1:1.23.2-1.module_2365+652bf990.x86_64
                         : libuv-devel-1:1.23.2-1.module_2365+652bf990.x86_64
                         : libuv-static-1:1.23.2-1.module_2365+652bf990.x86_64
                         : nghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                         : nodejs-1:12.1.0-1.module_2379+8d497405.src
                         : nodejs-1:12.1.0-1.module_2379+8d497405.x86_64
                         : nodejs-devel-1:12.1.0-1.module_2379+8d497405.x86_64
                         : nodejs-docs-1:12.1.0-1.module_2379+8d497405.noarch
                         : npm-1:12.1.0-1.module_2379+8d497405.x86_64

        Name             : nodejs
        Stream           : 5
        Version          : 20150811143428
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default, development, minimal
        Default profiles :
        Repo             : dnf-ci-fedora-modular
        Summary          : Javascript runtime module with quite a long
                         : summary that contains an empty line.
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
        Artifacts        : nodejs-1:5.3.1-1.module_2011+41787af0.src
                         : nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
                         : nodejs-devel-1:5.3.1-1.module_2011+41787af0.x86_64
                         : nodejs-docs-1:5.3.1-1.module_2011+41787af0.noarch
                         : npm-1:5.3.1-1.module_2011+41787af0.x86_64

        Name             : nodejs
        Stream           : 8 [d][e][a]
        Version          : 20180801080000
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default [d] [i], development, minimal
        Default profiles : default
        Repo             : dnf-ci-fedora-modular
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
        Artifacts        : nodejs-1:8.11.4-1.module_2030+42747d40.src
                         : nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch
                         : npm-1:8.11.4-1.module_2030+42747d40.x86_64

        Name             : nodejs
        Stream           : 8 [d][e][a]
        Version          : 20181216123422
        Context          : 7f892346
        Architecture     : x86_64
        Profiles         : default [d] [i], development, minimal
        Default profiles : default
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
        Artifacts        : nodejs-1:8.11.4-1.module_2030+42747d40.src
                         : nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch
                         : npm-1:8.14.0-1.module_2030+42747d41.x86_64

        Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
        """


  @bz1540189
  Scenario: Get info for an installed profile, module name, stream and profile specified
   When I execute dnf with args "module info nodejs:11/minimal"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Ignoring unnecessary profile: 'nodejs/minimal'
        Name             : nodejs
        Stream           : 11
        Version          : 20180920144611
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default, development, minimal
        Default profiles :
        Repo             : dnf-ci-fedora-modular
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
                         : postgresql:[9.6]
        Artifacts        : nodejs-1:11.0.0-1.module_2311+8d497411.src
                         : nodejs-1:11.0.0-1.module_2311+8d497411.x86_64
                         : nodejs-devel-1:11.0.0-1.module_2311+8d497411.x86_64
                         : nodejs-docs-1:11.0.0-1.module_2311+8d497411.noarch
                         : npm-1:11.0.0-1.module_2311+8d497411.x86_64

        Name             : nodejs
        Stream           : 11
        Version          : 20181102165620
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default, development, minimal
        Default profiles :
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
                         : postgresql:[9.6]
        Artifacts        : libnghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                         : libnghttp2-devel-0:1.34.0-1.module_2365+652bf990.x86_64
                         : libuv-1:1.23.2-1.module_2365+652bf990.x86_64
                         : libuv-devel-1:1.23.2-1.module_2365+652bf990.x86_64
                         : libuv-static-1:1.23.2-1.module_2365+652bf990.x86_64
                         : nghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                         : nodejs-1:11.1.0-1.module_2379+8d497405.src
                         : nodejs-1:11.1.0-1.module_2379+8d497405.x86_64
                         : nodejs-devel-1:11.1.0-1.module_2379+8d497405.x86_64
                         : nodejs-docs-1:11.1.0-1.module_2379+8d497405.noarch
                         : npm-1:11.1.0-1.module_2379+8d497405.x86_64

        Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
        """


  @bz1540189
  Scenario: Non-existent profile is ignored for dnf module info
   When I execute dnf with args "module info nodejs:11/non-existing-profile"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Ignoring unnecessary profile: 'nodejs/non-existing-profile'
        Name             : nodejs
        Stream           : 11
        Version          : 20180920144611
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default, development, minimal
        Default profiles :
        Repo             : dnf-ci-fedora-modular
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
                         : postgresql:[9.6]
        Artifacts        : nodejs-1:11.0.0-1.module_2311+8d497411.src
                         : nodejs-1:11.0.0-1.module_2311+8d497411.x86_64
                         : nodejs-devel-1:11.0.0-1.module_2311+8d497411.x86_64
                         : nodejs-docs-1:11.0.0-1.module_2311+8d497411.noarch
                         : npm-1:11.0.0-1.module_2311+8d497411.x86_64

        Name             : nodejs
        Stream           : 11
        Version          : 20181102165620
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default, development, minimal
        Default profiles :
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
                         : postgresql:[9.6]
        Artifacts        : libnghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                         : libnghttp2-devel-0:1.34.0-1.module_2365+652bf990.x86_64
                         : libuv-1:1.23.2-1.module_2365+652bf990.x86_64
                         : libuv-devel-1:1.23.2-1.module_2365+652bf990.x86_64
                         : libuv-static-1:1.23.2-1.module_2365+652bf990.x86_64
                         : nghttp2-0:1.34.0-1.module_2365+652bf990.x86_64
                         : nodejs-1:11.1.0-1.module_2379+8d497405.src
                         : nodejs-1:11.1.0-1.module_2379+8d497405.x86_64
                         : nodejs-devel-1:11.1.0-1.module_2379+8d497405.x86_64
                         : nodejs-docs-1:11.1.0-1.module_2379+8d497405.noarch
                         : npm-1:11.1.0-1.module_2379+8d497405.x86_64

        Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
        """


  @bz1623535
  Scenario: Get error message when info for non-existent module is requested
   When I execute dnf with args "module info non-existing-module"
   Then the exit code is 1
    And stdout contains "Unable to resolve argument non-existing-module"
    And stderr contains "Error: No matching Modules to list"


  Scenario: Get info for two enabled modules from different repos
   When I execute dnf with args "module info nodejs:8 postgresql:10"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name             : nodejs
        Stream           : 8 [d][e][a]
        Version          : 20180801080000
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default [d] [i], development, minimal
        Default profiles : default
        Repo             : dnf-ci-fedora-modular
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
        Artifacts        : nodejs-1:8.11.4-1.module_2030+42747d40.src
                         : nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch
                         : npm-1:8.11.4-1.module_2030+42747d40.x86_64

        Name             : nodejs
        Stream           : 8 [d][e][a]
        Version          : 20181216123422
        Context          : 7f892346
        Architecture     : x86_64
        Profiles         : default [d] [i], development, minimal
        Default profiles : default
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
        Artifacts        : nodejs-1:8.11.4-1.module_2030+42747d40.src
                         : nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch
                         : npm-1:8.14.0-1.module_2030+42747d41.x86_64

        Name             : postgresql
        Stream           : 10
        Version          : 20181211125304
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : client, default, server
        Default profiles :
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : PostgreSQL module
        Description      : PostgreSQL is an advanced Object-Relational database management system (DBMS). The PostgreSQL server can be found in the postgresql-server sub-package.
        Requires         : platform:[f29]
        Artifacts        : postgresql-0:10.6-1.module_2594+0c9aadc5.src
                         : postgresql-0:10.6-1.module_2594+0c9aadc5.x86_64
                         : postgresql-libs-0:10.6-1.module_2594+0c9aadc5.x86_64
                         : postgresql-server-0:10.6-1.module_2594+0c9aadc5.x86_64
                         : postgresql-test-0:10.6-1.module_2594+0c9aadc5.x86_64

        Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
        """


  @bz1623535
  # Command "dnf module info" should behave like "dnf info" in case that only one argument cannot
  # be resolved (success).
  Scenario: Get info for two modules, one of them non-existent
   When I execute dnf with args "module info postgresql:10 non-existing-module"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Unable to resolve argument non-existing-module
        Name             : postgresql
        Stream           : 10
        Version          : 20181211125304
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : client, default, server
        Default profiles :
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : PostgreSQL module
        Description      : PostgreSQL is an advanced Object-Relational database management system (DBMS). The PostgreSQL server can be found in the postgresql-server sub-package.
        Requires         : platform:[f29]
        Artifacts        : postgresql-0:10.6-1.module_2594+0c9aadc5.src
                         : postgresql-0:10.6-1.module_2594+0c9aadc5.x86_64
                         : postgresql-libs-0:10.6-1.module_2594+0c9aadc5.x86_64
                         : postgresql-server-0:10.6-1.module_2594+0c9aadc5.x86_64
                         : postgresql-test-0:10.6-1.module_2594+0c9aadc5.x86_64

        Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
        """


 Scenario: Run 'dnf module info' without further argument (dnf)
  Given I set dnf command to "dnf"
   When I execute dnf with args "module info"
   Then the exit code is 1
    And stderr contains "Error: dnf module info: too few arguments"

 Scenario: Run 'dnf module info' without further argument (yum)
  Given I set dnf command to "yum"
   When I execute dnf with args "module info"
   Then the exit code is 1
    And stderr contains "Error: yum module info: too few arguments"

  @bz1571214
  Scenario Outline: I can get the info about content of existing module streams with <command>
   When I execute dnf with args "<command>"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name    : postgresql:10:20181211125304:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server

        Name    : postgresql:11:20181212114126:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server

        Name    : postgresql:6:20190329142114:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server

        Name    : postgresql:9.6:20180816142114:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server

        Name    : postgresql:9.6:20190109151606:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server
        """

Examples:
    | command                               |
    | module info --profile postgresql      |
    | -q module info --profile postgresql   |



  Scenario: Profile specification is ignored by dnf module info --profile
   When I execute dnf with args "module info --profile postgresql/client"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Ignoring unnecessary profile: 'postgresql/client'
        Name    : postgresql:10:20181211125304:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server

        Name    : postgresql:11:20181212114126:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server

        Name    : postgresql:6:20190329142114:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server

        Name    : postgresql:9.6:20180816142114:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server

        Name    : postgresql:9.6:20190109151606:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server
        """


  Scenario: I can get the info about contents of more than one module profile streams
   When I execute dnf with args "module info --profile postgresql:10 nodejs:11"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name        : nodejs:11:20180920144611:6c81f848:x86_64
        default     : nodejs
                    : npm
        development : nodejs
                    : nodejs-devel
                    : npm
        minimal     : nodejs

        Name        : nodejs:11:20181102165620:6c81f848:x86_64
        default     : nodejs
                    : npm
        development : nodejs
                    : nodejs-devel
                    : npm
        minimal     : nodejs

        Name    : postgresql:10:20181211125304:6c81f848:x86_64
        client  : postgresql
        default : postgresql-server
        server  : postgresql-server
        """


  Scenario: "dnf module profile" without any additional arguments should raise an error
  Given I set dnf command to "dnf"
   When I execute dnf with args "module info --profile"
   Then the exit code is 1
    And stderr is
    """
    Error: dnf module info: too few arguments
    """


  @bz1636091
  Scenario: The module stream context information is present
   When I execute dnf with args "module info nodejs:11"
   Then the exit code is 0
   And stdout contains "Context\s+:\s+6c81f848"

  @bz1700250
  @bz1636337
  Scenario: I can get the module context of the active stream
   When I execute dnf with args "module info nodejs:8"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name             : nodejs
        Stream           : 8 [d][e][a]
        Version          : 20180801080000
        Context          : 6c81f848
        Architecture     : x86_64
        Profiles         : default [d] [i], development, minimal
        Default profiles : default
        Repo             : dnf-ci-fedora-modular
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
        Artifacts        : nodejs-1:8.11.4-1.module_2030+42747d40.src
                         : nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch
                         : npm-1:8.11.4-1.module_2030+42747d40.x86_64

        Name             : nodejs
        Stream           : 8 [d][e][a]
        Version          : 20181216123422
        Context          : 7f892346
        Architecture     : x86_64
        Profiles         : default [d] [i], development, minimal
        Default profiles : default
        Repo             : dnf-ci-fedora-modular-updates
        Summary          : Javascript runtime
        Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
        Requires         : platform:[f29]
        Artifacts        : nodejs-1:8.11.4-1.module_2030+42747d40.src
                         : nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
                         : nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch
                         : npm-1:8.14.0-1.module_2030+42747d41.x86_64

        Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
        """
