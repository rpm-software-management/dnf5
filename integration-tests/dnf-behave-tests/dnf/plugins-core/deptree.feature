Feature: Deptree command tests

# --arch=x86_64 is there to skip *.src rpms present in repos
Scenario: Forward tree uses the latest provider version
  Given I use repository "deptree"
   When I execute dnf with args "deptree trailhead --arch=x86_64"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    trailhead-1-1.fc29.x86_64
    └─ waypost-2-1.fc29.x86_64
       └─ campsite-1-1.fc29.x86_64
    """


Scenario: Depth limits forward traversal
  Given I use repository "deptree"
   When I execute dnf with args "deptree --depth=1 trailhead --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    trailhead-1-1.fc29.x86_64
    └─ waypost-2-1.fc29.x86_64
    """


Scenario: Reverse tree uses the latest dependent version
  Given I use repository "deptree"
   When I execute dnf with args "deptree --reverse campsite --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    campsite-1-1.fc29.x86_64
    └─ waypost-2-1.fc29.x86_64
       └─ trailhead-1-1.fc29.x86_64
    """


# junction Requires: (north-trail or south-trail)
Scenario: Rich OR requirements produce a choice node
  Given I use repository "deptree"
   When I execute dnf with args "deptree junction --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    junction-1-1.fc29.x86_64
    └─ choice: (north-trail or south-trail)
       ├─ north-trail-1-1.fc29.x86_64
       └─ south-trail-1-1.fc29.x86_64
    """


# nested-choice Requires: ((rope and tent) or map)
Scenario: Nested rich OR requirements retain every operand branch
  Given I use repository "deptree"
   When I execute dnf with args "deptree nested-choice --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    nested-choice-1-1.fc29.x86_64
    └─ choice: ((rope and tent) or map)
       ├─ rope-1-1.fc29.x86_64
       ├─ tent-1-1.fc29.x86_64
       └─ map-1-1.fc29.x86_64
    """


Scenario: Show requires renders requirement nodes
  Given I use repository "deptree"
   When I execute dnf with args "deptree --show-requires --depth=1 trailhead --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    trailhead-1-1.fc29.x86_64
    └─ requirement: waypost
       └─ waypost-2-1.fc29.x86_64
    """


Scenario: Showduplicates includes all provider versions
  Given I use repository "deptree"
   When I execute dnf with args "deptree --showduplicates --flat trailhead"
   Then the exit code is 0
    And stdout is
    """
    campsite-1-1.fc29.x86_64
    trailhead-1-1.fc29.src
    trailhead-1-1.fc29.x86_64
    waypost-1-1.fc29.x86_64
    waypost-2-1.fc29.x86_64
    """


Scenario: Arch limits roots and traversal candidates
  Given I use repository "deptree"
   When I execute dnf with args "deptree --showduplicates --arch=x86_64 --flat trailhead"
   Then the exit code is 0
    And stdout is
    """
    campsite-1-1.fc29.x86_64
    trailhead-1-1.fc29.x86_64
    waypost-1-1.fc29.x86_64
    waypost-2-1.fc29.x86_64
    """


Scenario: Installed limits the tree to installed packages
  Given I use repository "deptree"
    And I successfully execute dnf with args "install trailhead --exclude=waypost-2"
   When I execute dnf with args "deptree --installed trailhead"
   Then the exit code is 0
    And stdout is
    """
    trailhead-1-1.fc29.x86_64
    └─ waypost-1-1.fc29.x86_64
       └─ campsite-1-1.fc29.x86_64
    """


Scenario: Flat JSON output lists package NEVRAs
  Given I use repository "deptree"
   When I execute dnf with args "deptree --flat --json trailhead --arch=x86_64"
   Then the exit code is 0
    And stdout json matches
    """
    [
      "campsite-1-1.fc29.x86_64",
      "waypost-2-1.fc29.x86_64",
      "trailhead-1-1.fc29.x86_64"
    ]
    """


Scenario: JSON output describes the dependency graph
  Given I use repository "deptree"
   When I execute dnf with args "deptree --json --depth=1 trailhead --arch=x86_64"
   Then the exit code is 0
    And stdout json matches
    """
    {
      "roots": ["pkg:trailhead-1-1.fc29.x86_64"],
      "nodes": [
        {
          "id": "pkg:waypost-2-1.fc29.x86_64",
          "type": "package",
          "name": "waypost",
          "epoch": "0",
          "version": "2",
          "release": "1.fc29",
          "arch": "x86_64"
        },
        {
          "id": "pkg:trailhead-1-1.fc29.x86_64",
          "type": "package",
          "name": "trailhead",
          "epoch": "0",
          "version": "1",
          "release": "1.fc29",
          "arch": "x86_64"
        }
      ],
      "edges": [
        {
          "source": "pkg:trailhead-1-1.fc29.x86_64",
          "target": "pkg:waypost-2-1.fc29.x86_64",
          "requirements": ["waypost"]
        }
      ]
    }
    """

# expedition Requires: (compass or map); Recommends: compass
Scenario: Repeated expanded packages are marked as already shown
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends expedition --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    expedition-1-1.fc29.x86_64
    ├─ compass-1-1.fc29.x86_64
    │  └─ landmark-1-1.fc29.x86_64
    └─ choice: (compass or map)
       ├─ compass-1-1.fc29.x86_64 [already shown]
       └─ map-1-1.fc29.x86_64
    """


# expedition Requires: (compass or map); Recommends: compass
Scenario: Depth-limited repeated packages are not marked as already shown
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends --depth=1 expedition --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    expedition-1-1.fc29.x86_64
    ├─ compass-1-1.fc29.x86_64
    └─ choice: (compass or map)
       ├─ compass-1-1.fc29.x86_64
       └─ map-1-1.fc29.x86_64
    """


# forecast Requires: (tent if (sunrise or starlight))
Scenario: Conditional rich dependencies render an if node
  Given I use repository "deptree"
   When I execute dnf with args "deptree forecast --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    forecast-1-1.fc29.x86_64
    └─ if (sunrise or starlight):
       └─ tent-1-1.fc29.x86_64
    """


# route-plan Requires: (water and (tarp if weather))
# outlook Recommends: (north-trail if weather else south-trail)
Scenario: Conditions with the same label retain their own branches
  Given I use repository "deptree"
  When I execute dnf with args "deptree route-plan outlook --types=requires,recommends --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    outlook-1-1.fc29.x86_64
    └─ if weather:
       ├─ north-trail-1-1.fc29.x86_64
       └─ else:
          └─ south-trail-1-1.fc29.x86_64

    route-plan-1-1.fc29.x86_64
    ├─ water-1-1.fc29.x86_64
    └─ if weather:
       └─ tarp-1-1.fc29.x86_64
    """


# packing-list Requires: (rope and tent)
Scenario: AND rich dependencies retain each branch
  Given I use repository "deptree"
   When I execute dnf with args "deptree packing-list --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    packing-list-1-1.fc29.x86_64
    ├─ rope-1-1.fc29.x86_64
    └─ tent-1-1.fc29.x86_64
    """


# detour Enhances: (bridge unless flood)
Scenario: Unless rich dependencies treat their first operand as active
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=enhances detour --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    detour-1-1.fc29.x86_64
    └─ bridge-1-1.fc29.x86_64
    """


# outlook Recommends: (north-trail if weather else south-trail)
Scenario: IF ELSE rich dependencies render both branches
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=recommends outlook --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    outlook-1-1.fc29.x86_64
    └─ if weather:
       ├─ north-trail-1-1.fc29.x86_64
       └─ else:
          └─ south-trail-1-1.fc29.x86_64
    """


# crossroads Enhances: (bridge unless storm else tunnel)
Scenario: UNLESS ELSE rich dependencies render both branches
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=enhances crossroads --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    crossroads-1-1.fc29.x86_64
    └─ if storm:
       ├─ tunnel-1-1.fc29.x86_64
       └─ else:
          └─ bridge-1-1.fc29.x86_64
    """


# river-crossing Requires: (gear-rope with gear-tarp)
# river-crossing Requires: (bridge-pass without flood-pass)
Scenario: WITH and WITHOUT rich dependencies filter candidates
  Given I use repository "deptree"
   When I execute dnf with args "deptree --show-requires river-crossing --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    river-crossing-1-1.fc29.x86_64
    ├─ requirement: (bridge-pass without flood-pass)
    │  └─ bridge-pass-1-1.fc29.x86_64
    └─ requirement: (gear-rope with gear-tarp)
       └─ gear-cache-1-1.fc29.x86_64
    """

# nested-constraints Requires: (gear-rope with (gear-tarp or flood-pass))
# nested-constraints Requires: (bridge-pass without (flood-pass or storm-pass))
Scenario: Nested WITH and WITHOUT OR operands resolve providers
  Given I use repository "deptree"
   When I execute dnf with args "deptree --show-requires nested-constraints --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    nested-constraints-1-1.fc29.x86_64
    ├─ requirement: (bridge-pass without (flood-pass or storm-pass))
    │  └─ bridge-pass-1-1.fc29.x86_64
    └─ requirement: (gear-rope with (gear-tarp or flood-pass))
       └─ gear-cache-1-1.fc29.x86_64
    """

Scenario: Unsatisfied recommends remain a single terminal node with show requires
  Given I use repository "deptree"
   When I execute dnf with args "deptree --show-requires --types=recommends lost-signal --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    lost-signal-1-1.fc29.x86_64
    └─ unsatisfied: missing-signal
    """

Scenario: Ambiguous roots use the newest version by default
  Given I use repository "deptree"
   When I execute dnf with args "deptree --depth=0 waypost"
   Then the exit code is 0
    And stdout is
    """
    waypost-2-1.fc29.src

    waypost-2-1.fc29.x86_64
    """


Scenario: Showduplicates retains every matching root version
  Given I use repository "deptree"
   When I execute dnf with args "deptree --showduplicates --depth=0 waypost"
   Then the exit code is 0
    And stdout is
    """
    waypost-1-1.fc29.src

    waypost-1-1.fc29.x86_64

    waypost-2-1.fc29.src

    waypost-2-1.fc29.x86_64
    """


Scenario: Types can select weak dependencies without requires
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=recommends expedition --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    expedition-1-1.fc29.x86_64
    └─ compass-1-1.fc29.x86_64
    """


Scenario: Providers include every architecture by default
  Given I use repository "deptree"
   When I execute dnf with args "deptree viewpoint.x86_64"
   Then the exit code is 0
    And stdout is
    """
    viewpoint-1-1.fc29.x86_64
    └─ choice: binoculars
       ├─ binoculars-1-1.fc29.i686
       └─ binoculars-1-1.fc29.x86_64
    """


Scenario: Package globs select every matching root
  Given I use repository "deptree"
   When I execute dnf with args "deptree --depth=0 --arch=x86_64 '*-trail'"
   Then the exit code is 0
    And stdout is
    """
    north-trail-1-1.fc29.x86_64

    south-trail-1-1.fc29.x86_64
    """


Scenario: Exact NEVRA selects an older package version
  Given I use repository "deptree"
   When I execute dnf with args "deptree --depth=0 waypost-1-1.fc29.x86_64"
   Then the exit code is 0
    And stdout is
    """
    waypost-1-1.fc29.x86_64
    """


Scenario: Arch accepts a comma-separated list
  Given I use repository "deptree"
   When I execute dnf with args "deptree --flat --showduplicates --arch=x86_64,i686 viewpoint"
   Then the exit code is 0
    And stdout is
    """
    binoculars-1-1.fc29.i686
    binoculars-1-1.fc29.x86_64
    viewpoint-1-1.fc29.x86_64
    """


Scenario: Unmatched package specs are reported without suppressing matches
  Given I use repository "deptree"
   When I execute dnf with args "deptree --depth=0 trailhead lost-trail --arch=x86_64"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    No match for argument "lost-trail".
    """
    And stdout is
    """
    trailhead-1-1.fc29.x86_64
    """
