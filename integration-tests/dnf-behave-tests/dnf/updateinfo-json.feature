Feature: dnf advisory command with --json


Background:
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "install glibc flac"


Scenario: Listing available updates in json format
   When I use repository "dnf-ci-fedora-updates"
    And I execute dnf with args "advisory list --json"
   Then the exit code is 0
    And stdout json matches
    """
    [
      {
        "name":"FEDORA-2999:002-02",
        "type":"enhancement",
        "severity":"Moderate",
        "nevra":"flac-1.3.3-8.fc29.x86_64",
        "buildtime":1547683200
      },
      {
        "name":"FEDORA-2018-318f184000",
        "type":"bugfix",
        "severity":"none",
        "nevra":"glibc-2.28-26.fc29.x86_64",
        "buildtime":1547683200
      }
    ]
    """


Scenario: Listing available updates referencing bugizilla in json format
   When I use repository "dnf-ci-fedora-updates"
    And I execute dnf with args "advisory list --with-bz --json"
   Then the exit code is 0
    And stdout json matches
    """
    [
      {
        "advisory_name":"FEDORA-2018-318f184000",
        "advisory_type":"bugfix",
        "advisory_severity":"bugfix",
        "advisory_buildtime":1547683200,
        "nevra":"glibc-2.28-26.fc29.x86_64",
        "references":[
          {
            "reference_id":"222",
            "reference_type":"bugzilla"
          }
        ]
      }
    ]
    """


Scenario: Listing updates in json format (when there's nothing to report)
   When I execute dnf with args "advisory list --json"
   Then the exit code is 0
    And stdout is
    """
    []
    """


Scenario: Listing updates in json format with custom type and severity
  Given I use repository "advisories-base"
    And I execute dnf with args "install labirinto"
    And I use repository "advisories-updates"
   When I execute dnf with args "updateinfo list --json"
   Then the exit code is 0
    And stdout json matches
    """
    [
      {
        "name":"FEDORA-2019-57b5902ed1",
        "type":"security",
        "severity":"Critical",
        "nevra":"labirinto-1.56.2-6.fc30.x86_64",
        "buildtime":1568511269
      },
      {
        "name":"FEDORA-2022-2222222222",
        "type":"custom_type",
        "severity":"custom_severity",
        "nevra":"labirinto-1.56.2-6.fc30.x86_64",
        "buildtime":1568511269
      },
      {
        "name":"FEDORA-2022-2222222223",
        "type":"security",
        "severity":"custom_severity",
        "nevra":"labirinto-1.56.2-6.fc30.x86_64",
        "buildtime":1568511269
      },
      {
        "name":"FEDORA-2022-2222222224",
        "type":"custom_type",
        "severity":"Critical",
        "nevra":"labirinto-1.56.2-6.fc30.x86_64",
        "buildtime":1568511269
      },
      {
        "name":"FEDORA-2019-f4eb34cf4c",
        "type":"security",
        "severity":"Moderate",
        "nevra":"labirinto-1.56.2-1.fc30.x86_64",
        "buildtime":1557624103
      }
    ]
    """


Scenario: Info about available updates in json format
   When I use repository "dnf-ci-fedora-updates"
    And I execute dnf with args "updateinfo info --json"
   Then the exit code is 0
    And stdout json matches
    """
    [
        {
          "Name":"FEDORA-2018-318f184000",
          "Title":"glibc bug fix",
          "Severity":"none",
          "Type":"bugfix",
          "Status":"final",
          "Vendor":"secresponseteam@foo.bar",
          "Issued":1547683200,
          "Description":"Fix some stuff",
          "Message":"",
          "Rights":"",
          "references":[
            {
              "Title":"222",
              "Id":"222",
              "Type":"bugzilla",
              "Url":"https:\/\/foobar\/foobarupdate_1"
            },
            {
              "Title":"CVE-2999",
              "Id":"2999",
              "Type":"cve",
              "Url":"https:\/\/foobar\/foobarupdate_1"
            },
            {
              "Title":"CVE-2999",
              "Id":"CVE-2999",
              "Type":"cve",
              "Url":"https:\/\/foobar\/foobarupdate_1"
            }
          ],
          "collections":{
            "packages":[
              "glibc-2.28-26.fc29.x86_64"
            ]
          }
        },
        {
          "Name":"FEDORA-2999:002-02",
          "Title":"flac enhacements",
          "Severity":"Moderate",
          "Type":"enhancement",
          "Status":"final",
          "Vendor":"secresponseteam@foo.bar",
          "Issued":1547683200,
          "Description":"Enhance some stuff",
          "Message":"",
          "Rights":"",
          "references":[
            {
              "Title":"update_1",
              "Id":"1",
              "Type":"self",
              "Url":"https:\/\/foobar\/foobarupdate_1"
            }
          ],
          "collections":{
            "packages":[
              "flac-1.3.3-8.fc29.x86_64"
            ]
          }
        }
      ]
    """


Scenario: Info about available updates referencing bugizilla in json format
   When I use repository "dnf-ci-fedora-updates"
    And I execute dnf with args "updateinfo info --with-bz --json"
   Then the exit code is 0
    And stdout json matches
    """
    [
        {
          "Name":"FEDORA-2018-318f184000",
          "Title":"glibc bug fix",
          "Severity":"none",
          "Type":"bugfix",
          "Status":"final",
          "Vendor":"secresponseteam@foo.bar",
          "Issued":1547683200,
          "Description":"Fix some stuff",
          "Message":"",
          "Rights":"",
          "references":[
            {
              "Title":"222",
              "Id":"222",
              "Type":"bugzilla",
              "Url":"https:\/\/foobar\/foobarupdate_1"
            },
            {
              "Title":"CVE-2999",
              "Id":"2999",
              "Type":"cve",
              "Url":"https:\/\/foobar\/foobarupdate_1"
            },
            {
              "Title":"CVE-2999",
              "Id":"CVE-2999",
              "Type":"cve",
              "Url":"https:\/\/foobar\/foobarupdate_1"
            }
          ],
          "collections":{
            "packages":[
              "glibc-2.28-26.fc29.x86_64"
            ]
          }
        }
      ]
    """


Scenario: Info about updates in json format (when there's nothing to report)
   When I execute dnf with args "updateinfo info --json"
   Then the exit code is 0
    And stdout is
    """
    []
    """


Scenario: Info about updates in json format with custom type and severity
  Given I use repository "advisories-base"
    And I execute dnf with args "install labirinto"
    And I use repository "advisories-updates"
   When I execute dnf with args "updateinfo info --json"
   Then the exit code is 0
    And stdout json matches
    """
    [
        {
          "Name":"FEDORA-2019-f4eb34cf4c",
          "Title":"labirinto-1.56.2-1.fc30",
          "Severity":"Moderate",
          "Type":"security",
          "Status":"stable",
          "Vendor":"updates@fedoraproject.org",
          "Issued":1557624103,
          "Description":"GNOME 3.32.2",
          "Message":"",
          "Rights":"Copyright (C) 2020 Red Hat, Inc. and others.",
          "references":[
            {
              "Title":"[[]abrt[]] epiphany: ephy_suggestion_get_unescaped_title(): epiphany-search-provider killed by SIGABRT",
              "Id":"1696529",
              "Type":"bugzilla",
              "Url":"https:\/\/bugzilla.redhat.com\/show_bug.cgi?id=1696529"
            }
          ],
          "collections":{
            "packages":[
              "labirinto-1.56.2-1.fc30.i686",
              "labirinto-1.56.2-1.fc30.x86_64",
              "labirinto-1.56.2-1.fc30.src"
            ]
          }
        },
        {
          "Name":"FEDORA-2019-57b5902ed1",
          "Title":"labirinto-1.56.2-6.fc30 mozjs60-60.9.0-2.fc30 polkit-0.116-2.fc30",
          "Severity":"Critical",
          "Type":"security",
          "Status":"stable",
          "Vendor":"updates@fedoraproject.org",
          "Issued":1568511269,
          "Description":"mozjs60 60.9.0, including various security, stability and regression fixes from Firefox 60.9.0 ESR. For details, see https:\/\/www.mozilla.org\/en-US\/firefox\/60.9.0\/releasenotes\/",
          "Message":"",
          "Rights":"Copyright (C) 2020 Red Hat, Inc. and others.",
          "references":[],
          "collections":{
            "packages":[
              "labirinto-1.56.2-6.fc30.i686",
              "labirinto-1.56.2-6.fc30.src",
              "labirinto-1.56.2-6.fc30.x86_64"
            ]
          }
        },
        {
          "Name":"FEDORA-2022-2222222222",
          "Title":"labirinto-1.56.2-6.fc30 mozjs60-60.9.0-2.fc30 polkit-0.116-2.fc30",
          "Severity":"custom_severity",
          "Type":"custom_type",
          "Status":"stable",
          "Vendor":"updates@fedoraproject.org",
          "Issued":1568511269,
          "Description":"advisory with custom type and seveirity",
          "Message":"",
          "Rights":"Copyright (C) 2020 Red Hat, Inc. and others.",
          "references":[],
          "collections":{
            "packages":[
              "labirinto-1.56.2-6.fc30.i686",
              "labirinto-1.56.2-6.fc30.src",
              "labirinto-1.56.2-6.fc30.x86_64"
            ]
          }
        },
        {
          "Name":"FEDORA-2022-2222222223",
          "Title":"labirinto-1.56.2-6.fc30 mozjs60-60.9.0-2.fc30 polkit-0.116-2.fc30",
          "Severity":"custom_severity",
          "Type":"security",
          "Status":"stable",
          "Vendor":"updates@fedoraproject.org",
          "Issued":1568511269,
          "Description":"advisory with custom seveirity",
          "Message":"",
          "Rights":"Copyright (C) 2020 Red Hat, Inc. and others.",
          "references":[],
          "collections":{
            "packages":[
              "labirinto-1.56.2-6.fc30.i686",
              "labirinto-1.56.2-6.fc30.src",
              "labirinto-1.56.2-6.fc30.x86_64"
            ]
          }
        },
        {
          "Name":"FEDORA-2022-2222222224",
          "Title":"labirinto-1.56.2-6.fc30 mozjs60-60.9.0-2.fc30 polkit-0.116-2.fc30",
          "Severity":"Critical",
          "Type":"custom_type",
          "Status":"stable",
          "Vendor":"updates@fedoraproject.org",
          "Issued":1568511269,
          "Description":"advisory with custom type",
          "Message":"",
          "Rights":"Copyright (C) 2020 Red Hat, Inc. and others.",
          "references":[],
          "collections":{
            "packages":[
              "labirinto-1.56.2-6.fc30.i686",
              "labirinto-1.56.2-6.fc30.src",
              "labirinto-1.56.2-6.fc30.x86_64"
            ]
          }
        }
      ]
    """
