# missing modularity features: https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Fail-safe


Background: Copy the dnf-ci-fedora-modular repo (to allow modulemd removal and so on), and enable nodejs:5 (containing the oldest nodejs package) -> this makes fail-safe copy of nodejs:5
  Given I copy repository "dnf-ci-fedora-modular" for modification
    And I use repository "dnf-ci-fedora-modular" with configuration
        | key                 | value |
        | skip_if_unavailable | 1     |
    And I configure repository "dnf-ci-fedora-modular-hotfix" with
        | key             | value |
        | module_hotfixes | 1     |
    And I configure repository "fail-safe-hotfix" with
        | key             | value |
        | module_hotfixes | 1     |
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "module enable nodejs:5"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | nodejs:5               |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |


# nodejs RPMs in used repositories (+ info about module streams):
#     dnf-ci-fedora:
#         nodejs-1:5.12.1-1.fc29.x86_64
#     dnf-ci-fedora-modular:
#         nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
#         nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
#         nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64
#         nodejs-1:11.0.0-1.module_2311+8d497411.x86_64
#         - nodejs:5 - nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
#         - nodejs:8 - nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
#         - nodejs:10 - nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64
#         - nodejs:11 - nodejs-1:11.0.0-1.module_2311+8d497411.x86_64
#     dnf-ci-fedora-modular-updates:
#         nodejs-1:10.14.1-1.module_2533+7361f245.x86_64
#         nodejs-1:11.1.0-1.module_2379+8d497405.x86_64
#         nodejs-1:12.1.0-1.module_2379+8d497405.x86_64
#         nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
#         nodejs-1:8.14.0-1.x86_64
#         - nodejs:8 - nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
#         - nodejs:10 - nodejs-1:10.14.1-1.module_2533+7361f245.x86_64
#         - nodejs:11 - nodejs-1:11.1.0-1.module_2379+8d497405.x86_64
#         - nodejs:12 - nodejs-1:12.1.0-1.module_2379+8d497405.x86_64
#     dnf-ci-fedora-modular-hotfix:
#         nodejs-1:8.11.5-1.module_2030+42747d40.x86_64
#     dnf-ci-fourthparty-modular:
#         nodejs-1:8.14.0-1.module_2030+42747d40.x86_64
#         - fake-nodejs - nodejs-1:8.14.0-1.module_2030+42747d40.x86_64


@bz1616167
@bz1623128
Scenario: When modulemd is available, 'module list' does NOT show fail-safe modules
  Given I use repository "dnf-ci-fedora-modular-updates"
   When I execute dnf with args "module enable postgresql:10"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | postgresql:10          |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |
        | postgresql     | enabled   | 10        |           |
   When I execute dnf with args "module list"
   Then the exit code is 0
   And module list is
       | Repository            | Name      | Stream     | Profiles                              |
       | dnf-ci-fedora-modular | meson     | master [d] | default [d]                           |
       | dnf-ci-fedora-modular | nodejs    | 5 [e]      | development, minimal, default         |
       | dnf-ci-fedora-modular | nodejs    | 8 [d]      | development, minimal, default [d]     |
       | dnf-ci-fedora-modular | nodejs    | 10         | development, minimal, default [d]     |
       | dnf-ci-fedora-modular | nodejs    | 11         | development, minimal, default         |
       | dnf-ci-fedora-modular | postgresql| 9.6 [d]    | client, server, default [d]           |
       | dnf-ci-fedora-modular | postgresql| 6          | client, server, default               |
       | dnf-ci-fedora-modular | ninja     | master [d] | default [d]                           |
       | dnf-ci-fedora-modular | ninja     | development| default [d]                           |
       | dnf-ci-fedora-modular | ninja     | legacy     | default                               |
       | dnf-ci-fedora-modular | dwm       | 6.0        | default                               |
       | dnf-ci-fedora-modular-updates | nodejs        | 8 [d]     | development, minimal, default [d] |
       | dnf-ci-fedora-modular-updates | nodejs        | 10        | development, minimal, default [d] |
       | dnf-ci-fedora-modular-updates | nodejs        | 11        | development, minimal, default     |
       | dnf-ci-fedora-modular-updates | nodejs        | 12        | development, minimal, default     |
       | dnf-ci-fedora-modular-updates | postgresql    | 9.6 [d]   | client, server, default [d]       |
       | dnf-ci-fedora-modular-updates | postgresql    | 10 [e]    | client, server, default           |
       | dnf-ci-fedora-modular-updates | postgresql    | 11        | client, server, default           |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, 'module list' shows fail-safe modules
  Given I use repository "dnf-ci-fedora-modular-updates"
   When I execute dnf with args "module enable postgresql:10"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | postgresql:10          |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |
        | postgresql     | enabled   | 10        |           |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "module list"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream    | Profiles                      |
        | dnf-ci-fedora-modular-updates | nodejs        | 8         | development, minimal, default |
        | dnf-ci-fedora-modular-updates | nodejs        | 10        | development, minimal, default |
        | dnf-ci-fedora-modular-updates | nodejs        | 11        | development, minimal, default |
        | dnf-ci-fedora-modular-updates | nodejs        | 12        | development, minimal, default |
        | dnf-ci-fedora-modular-updates | postgresql    | 9.6       | client, server, default       |
        | dnf-ci-fedora-modular-updates | postgresql    | 10 [e]    | client, server, default       |
        | dnf-ci-fedora-modular-updates | postgresql    | 11        | client, server, default       |
        | @modulefailsafe               | nodejs        | 5 [e]     | development, default, minimal |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, 'module info' can show details for fail-safe modules
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "module info nodejs:5"
   Then the exit code is 0
    And stdout contains "Name.*nodejs"
    And stdout contains "Stream.*5"

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, module profile from enabled stream cannot be installed
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "module install nodejs:5/default"
   Then the exit code is 1
    And Transaction is empty

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, module profile from non-enabled stream cannot be installed
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "module install nodejs:8/default"
   Then the exit code is 1
    And Transaction is empty

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, module profile from enabled stream can be removed
   When I execute dnf with args "module install nodejs:5/minimal"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                        |
        | install-group             | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64   |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         | minimal   |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "module remove nodejs:5/minimal"
   Then the exit code is 0
    And Transaction contains
        | Action         | Package                                          |
        | remove         | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64     |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, repoquery doesn't show RPMs with the same name as in the enabled stream (non-modular or from different streams or modules), only from hotfix repos
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
    And I use repository "dnf-ci-fedora-modular-updates"
    And I use repository "dnf-ci-fourthparty-modular"
   When I execute dnf with args "repoquery nodejs.x86_64"
   Then the exit code is 0
    And stdout is empty
   When I use repository "fail-safe-hotfix"
    And I use repository "dnf-ci-fedora-modular-hotfix"
    And I execute dnf with args "repoquery nodejs.x86_64"
   Then the exit code is 0
    And stdout contains "nodejs-1:5.12.1-1.fc29.x86_64"
    And stdout contains "nodejs-1:8.11.5-1.module_2030\+42747d40.x86_64"

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modular RPM is installed and modulemd is not available, the RPM can't be upgraded to non-modular RPM nor an RPM from different stream or module
   When I execute dnf with args "install nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                            |
        | install                   | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64       |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
    And I use repository "dnf-ci-fedora-modular-updates"
    And I use repository "dnf-ci-fourthparty-modular"
   When I execute dnf with args "upgrade nodejs"
   Then the exit code is 0
    And Transaction is empty
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modular RPM is installed and modulemd is not available, the RPM can be upgraded to modular RPM from hotfix repository
   When I execute dnf with args "install nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                            |
        | install                   | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64       |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
    And I use repository "dnf-ci-fedora-modular-hotfix"
   When I execute dnf with args "upgrade nodejs"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                            |
        | upgrade                   | nodejs-1:8.11.5-1.module_2030+42747d40.x86_64      |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modular RPM is installed and modulemd is not available, the RPM can be upgraded to non-modular RPM from hotfix repository
   When I execute dnf with args "install nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                            |
        | install                   | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64       |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
    And I use repository "fail-safe-hotfix"
   When I execute dnf with args "upgrade nodejs"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                            |
        | upgrade                   | nodejs-1:5.12.1-1.fc29.x86_64                      |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modular RPM is installed and modulemd is not available, the RPM can be upgraded to RPM from commandline
   When I execute dnf with args "install nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                            |
        | install                   | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64       |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "upgrade {context.scenario.repos_location}/dnf-ci-fedora-modular-updates/x86_64/nodejs-8.11.4-1.module_2030+42747d40.x86_64.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                                            |
        | upgrade               | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64      |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When non-modular RPM is installed and modulemd is not available, the RPM can't be upgraded to an RPM from different stream or module
   When I execute dnf with args "install http-parser-2.4.0-1.fc29"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                            |
        | install                   | http-parser-0:2.4.0-1.fc29.x86_64                  |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
    And I use repository "dnf-ci-fedora-modular-updates"
    And I use repository "dnf-ci-fourthparty-modular"
   When I execute dnf with args "upgrade http-parser"
   Then the exit code is 0
    And Transaction is empty
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When non-modular RPM is installed and modulemd is not available, the RPM can be upgraded to an RPM from hotfix repository
   When I execute dnf with args "install http-parser-2.4.0-1.fc29"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                            |
        | install                   | http-parser-0:2.4.0-1.fc29.x86_64                  |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
    And I use repository "dnf-ci-fedora-modular-hotfix"
   When I execute dnf with args "upgrade http-parser"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                            |
        | upgrade                   | http-parser-0:2.4.0-2.module_2672+97d6a5e9.x86_64  |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When non-modular RPM is installed and modulemd is not available, the RPM can be upgraded to an RPM from commandline
   When I execute dnf with args "install http-parser-2.4.0-1.fc29"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                            |
        | install                   | http-parser-0:2.4.0-1.fc29.x86_64                  |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "upgrade {context.scenario.repos_location}/dnf-ci-fedora-modular-updates/x86_64/http-parser-2.9.0-1.module_2672+97d6a5e9.x86_64.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                                            |
        | upgrade               | http-parser-0:2.9.0-1.module_2672+97d6a5e9.x86_64  |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, non-modular RPMs can be installed and upgraded if their name differs from packages in enabled stream
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "install wget"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                            |
        | install               | wget-0:1.19.5-5.fc29.x86_64        |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade wget"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                            |
        | upgrade               | wget-0:1.19.6-5.fc29.x86_64        |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, the enabled module can be disabled
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "module disable nodejs"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                            |
        | module-disable        | nodejs                             |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | disabled  |           |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, the enabled module can be reset
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "module reset nodejs"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                            |
        | module-reset          | nodejs                             |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         |           |           |           |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, RPM from the enabled stream can be installed
   When I execute dnf with args "module reset nodejs"
    And I execute dnf with args "module enable nodejs:10"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | nodejs:10              |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 10        |           |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
      # Workaround for a bug - when the repo id is too log and nodejs is installed later on, the output from transaction is formatted incorrectly (no space between "Arch" and "Version")
    And I configure a new repository "short-modular-updates" with
        | key     | value                                                      |
        | baseurl | {context.scenario.repos_location}/dnf-ci-fedora-modular-updates |
      # since we're configuring the repo ourselves, force generation of repodata by using and unusing it
    And I use repository "dnf-ci-fedora-modular-updates"
    And I drop repository "dnf-ci-fedora-modular-updates"
   When I execute dnf with args "install nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action         | Package                                          |
        | install        | nodejs-1:10.14.1-1.module_2533+7361f245.x86_64   |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: When modulemd is not available, RPM from the enabled stream can be removed
   When I execute dnf with args "install nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                        |
        | install                   | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64   |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |
  Given I execute step "<step>"
    And I execute dnf with args "clean all"
   When I execute dnf with args "remove nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action         | Package                                          |
        | remove         | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64     |

Examples:
    | step                                                                                                      |
    | Given I drop repository "dnf-ci-fedora-modular"                                                           |
    | Given I delete directory "/{context.dnf.repos[dnf-ci-fedora-modular].path}"                               |
    | Given I delete file "/{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata/*modules.yaml*" with globs |
