Feature: Deptree command tests

# --arch=x86_64 is there to skip *.src rpms present in repos
# trailhead Requires waypost; waypost versions 1 and 2 both Require campsite.
# The default provider selection must choose waypost-2.
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


# trailhead -> waypost -> campsite has package depths 0, 1, 2.
# With depth=1, waypost is included but its campsite dependency is not followed.
Scenario: Depth limits forward traversal
  Given I use repository "deptree"
   When I execute dnf with args "deptree --depth=1 trailhead --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    trailhead-1-1.fc29.x86_64
    └─ waypost-2-1.fc29.x86_64
    """


# basecamp Recommends: approach; Requires: trailhead; approach Recommends: trailhead
# trailhead Requires: waypost; waypost Requires: campsite
# Recommendations precede requirements, putting the longer path first in the tree.
# campsite is the key output line: its shortest depth is 3, but the path through
# approach reaches it at depth 4.
Scenario: Depth-limited traversal expands shared nodes at their shallowest package depth
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends --depth=3 --flat --show-requires basecamp --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    approach-1-1.fc29.x86_64
    basecamp-1-1.fc29.x86_64
    campsite-1-1.fc29.x86_64
    trailhead-1-1.fc29.x86_64
    waypost-2-1.fc29.x86_64
    """


# basecamp reaches trailhead through approach (depth 2) and directly (depth 1).
# Defer the first occurrence so waypost and campsite expand below the direct one.
Scenario: Shared subtrees expand at their minimum depth even when a longer path is printed first
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends --depth=3 basecamp --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    basecamp-1-1.fc29.x86_64
    ├─ approach-1-1.fc29.x86_64
    │  └─ trailhead-1-1.fc29.x86_64 [expanded elsewhere]
    └─ trailhead-1-1.fc29.x86_64
       └─ waypost-2-1.fc29.x86_64
          └─ campsite-1-1.fc29.x86_64
    """


# basecamp -> trailhead -> waypost -> campsite still has package depths 0, 1, 2, 3
# when requirement labels add extra tree levels. The path through approach is longer.
Scenario: Requirement nodes do not affect minimum package depth during rendering
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends --show-requires --depth=3 basecamp --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    basecamp-1-1.fc29.x86_64
    ├─ requirement: approach
    │  └─ approach-1-1.fc29.x86_64
    │     └─ requirement: trailhead
    │        └─ trailhead-1-1.fc29.x86_64 [expanded elsewhere]
    └─ requirement: trailhead
       └─ trailhead-1-1.fc29.x86_64
          └─ requirement: waypost
             └─ waypost-2-1.fc29.x86_64
                └─ requirement: campsite
                   └─ campsite-1-1.fc29.x86_64
    """


# basecamp Recommends approach, which Recommends trailhead; basecamp also Requires trailhead.
# trailhead -> waypost -> campsite is a Requires chain. Selecting trailhead as a root
# gives it depth 0, so both occurrences under basecamp defer to that later root.
# campsite is then within depth=2, although it is at depth 3 from basecamp.
Scenario: Package expansion can be deferred to a later selected root
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends --depth=2 basecamp trailhead --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    basecamp-1-1.fc29.x86_64
    ├─ approach-1-1.fc29.x86_64
    │  └─ trailhead-1-1.fc29.x86_64 [expanded elsewhere]
    └─ trailhead-1-1.fc29.x86_64 [expanded elsewhere]

    trailhead-1-1.fc29.x86_64
    └─ waypost-2-1.fc29.x86_64
       └─ campsite-1-1.fc29.x86_64
    """


# approach Recommends trailhead; basecamp Requires trailhead and Recommends approach.
# Both roots reach trailhead at depth 1. The first root, approach, expands its
# waypost -> campsite subtree; basecamp shares that expansion instead of repeating it.
# approach itself is also a root, so its occurrence under basecamp is a reference.
Scenario: Equally short occurrences share expansion across selected roots
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends --depth=3 approach basecamp --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    approach-1-1.fc29.x86_64
    └─ trailhead-1-1.fc29.x86_64
       └─ waypost-2-1.fc29.x86_64
          └─ campsite-1-1.fc29.x86_64

    basecamp-1-1.fc29.x86_64
    ├─ approach-1-1.fc29.x86_64 [expanded elsewhere]
    └─ trailhead-1-1.fc29.x86_64 [expanded elsewhere]
    """


# Forward Requires chain: trailhead -> waypost -> campsite. approach Recommends
# trailhead, and basecamp Requires it, so both appear as reverse dependents of trailhead.
# Selecting waypost as a root defers its occurrence under campsite; its own tree
# reaches trailhead at depth 1 and approach/basecamp at depth 2.
Scenario: Reverse trees share expansion across selected roots
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends --reverse --depth=2 campsite waypost --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    campsite-1-1.fc29.x86_64
    └─ waypost-2-1.fc29.x86_64 [expanded elsewhere]

    waypost-2-1.fc29.x86_64
    └─ trailhead-1-1.fc29.x86_64
       ├─ approach-1-1.fc29.x86_64
       └─ basecamp-1-1.fc29.x86_64
    """


# loop-a Recommends loop-b, and loop-b Recommends loop-a.
# The return to loop-a is at depth 2, while the root is at depth 0; stop at a reference.
Scenario: Cycles render references to package subtrees expanded at a shorter depth
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=recommends loop-a --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    loop-a-1-1.fc29.x86_64
    └─ loop-b-1-1.fc29.x86_64
       └─ loop-a-1-1.fc29.x86_64 [expanded elsewhere]
    """


# basecamp -> trailhead -> waypost -> campsite is the shortest path to the match.
# The alternative basecamp -> approach -> trailhead path adds a package level,
# so pruning removes that longer route and approach even though it also leads to campsite.
Scenario: Pruned graphs retain minimum-depth expansion paths
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends --contains-pkgs=campsite --depth=3 basecamp --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    basecamp-1-1.fc29.x86_64
    └─ trailhead-1-1.fc29.x86_64
       └─ waypost-2-1.fc29.x86_64
          └─ campsite-1-1.fc29.x86_64
    """


# Both waypost versions Require campsite; trailhead Requires waypost.
# Reverse traversal must keep waypost-2, then find trailhead as its dependent.
# basecamp Requires trailhead, so the reverse chain continues through basecamp.
Scenario: Reverse tree uses the latest dependent version
  Given I use repository "deptree"
   When I execute dnf with args "deptree --reverse campsite --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    campsite-1-1.fc29.x86_64
    └─ waypost-2-1.fc29.x86_64
       └─ trailhead-1-1.fc29.x86_64
          └─ basecamp-1-1.fc29.x86_64
    """


# junction Requires: (north-trail or south-trail)
# Both operand packages exist; display both alternatives under the choice node.
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
# rope, tent, and map all exist. The left alternative is a conjunction, so retain
# both rope and tent alongside the map alternative when unpacking the outer OR.
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


# trailhead Requires waypost; versions 1 and 2 both Require campsite.
# The repository also contains a trailhead.src root. Keep both provider versions
# and both root architectures, but list the shared campsite package only once.
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


# viewpoint Requires binoculars, which has x86_64 and i686 builds.
# The architecture filter excludes viewpoint.src as a root and binoculars.i686 as a provider.
Scenario: Arch limits roots and traversal candidates
  Given I use repository "deptree"
   When I execute dnf with args "deptree --arch=x86_64 viewpoint"
   Then the exit code is 0
    And stdout is
    """
    viewpoint-1-1.fc29.x86_64
    └─ binoculars-1-1.fc29.x86_64
    """


# trailhead Requires waypost; versions 1 and 2 both Require campsite.
# Excluding waypost-2 during installation leaves waypost-1 installed. The tree
# must follow that installed version even though the repository contains version 2.
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


# trailhead -> waypost-2 -> campsite is the selected Requires chain.
# Flat JSON contains the three package NEVRAs as array entries.
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


# trailhead Requires waypost; waypost-2 is the latest provider and Requires campsite.
# At depth=1, JSON includes the root, waypost, and their capability-labelled edge;
# campsite is beyond the limit and must be absent from both nodes and edges.
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
# compass Requires landmark. Both compass occurrences have package depth 1;
# the recommendation is printed first and expands landmark, while the OR branch references it.
Scenario: Shared packages expand once across dependency types
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=requires,recommends expedition --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    expedition-1-1.fc29.x86_64
    ├─ compass-1-1.fc29.x86_64
    │  └─ landmark-1-1.fc29.x86_64
    └─ choice: (compass or map)
       ├─ compass-1-1.fc29.x86_64 [expanded elsewhere]
       └─ map-1-1.fc29.x86_64
    """


# expedition Requires: (compass or map); Recommends: compass
# compass Requires landmark, but landmark is at depth 2 and excluded here.
# Both compass occurrences are therefore leaves with no subtree to mark as expanded elsewhere.
Scenario: Depth-limited leaf packages are not marked as expanded elsewhere
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
# tent exists; sunrise and starlight have no providers. The condition remains a
# label on the structural node, and tent is the dependency displayed below it.
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
# Both conditions display "if weather", but belong to different expressions.
# outlook has north-trail/south-trail alternatives; route-plan has only tarp under
# the condition, with water as a sibling. Keep the conditional nodes distinct.
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
# Both packages exist; the conjunction produces two required sibling branches.
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
# bridge exists; flood is a suppressing condition with no provider.
# The dependency shown for this expression is bridge.
Scenario: Unless rich dependencies treat their first operand as active
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=enhances detour --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    detour-1-1.fc29.x86_64
    └─ bridge-1-1.fc29.x86_64
    """


# crossroads Enhances: (bridge unless storm else tunnel)
# bridge and tunnel exist; storm is the condition label. UNLESS ELSE places
# tunnel under "if storm" and bridge under "else", reversing the operand placement of IF ELSE.
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
# gear-cache-1 provides gear-rope and gear-tarp; gear-cache-2 provides only gear-rope.
# WITH must keep version 1 because one package must provide both capabilities.
# bridge-pass-1 provides bridge-pass only; bridge-pass-2 and flooded-pass provide
# both bridge-pass and flood-pass. WITHOUT removes the latter two, leaving version 1.
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
# gear-cache-1 provides gear-rope and gear-tarp; gear-cache-2 provides only gear-rope.
# bridge-pass-1 provides bridge-pass only; bridge-pass-2 and flooded-pass also provide
# flood-pass. storm-marker provides storm-pass. Resolve each nested OR to its
# providers before filtering: WITH keeps gear-cache-1, and WITHOUT keeps bridge-pass-1.
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

# lost-signal Recommends missing-signal, which has no provider in the repository.
# Even with --show-requires, print one unsatisfied leaf for that capability.
Scenario: Unsatisfied recommends remain a single terminal node with show requires
  Given I use repository "deptree"
   When I execute dnf with args "deptree --show-requires --types=recommends lost-signal --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    lost-signal-1-1.fc29.x86_64
    └─ unsatisfied: missing-signal
    """

# waypost versions 1 and 2 each have src and x86_64 builds.
# An unqualified name selects version 2 for each architecture; depth=0 isolates root selection.
Scenario: Ambiguous roots use the newest version by default
  Given I use repository "deptree"
   When I execute dnf with args "deptree --depth=0 waypost"
   Then the exit code is 0
    And stdout is
    """
    waypost-2-1.fc29.src

    waypost-2-1.fc29.x86_64
    """


# waypost versions 1 and 2 each have src and x86_64 builds.
# --showduplicates keeps all four roots; depth=0 prevents their dependencies from obscuring selection.
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


# expedition Recommends compass and Requires (compass or map); compass Requires landmark.
# Following only recommendations includes compass, while omitting both the OR
# requirement on expedition and the landmark requirement on compass.
Scenario: Types can select weak dependencies without requires
  Given I use repository "deptree"
   When I execute dnf with args "deptree --types=recommends expedition --arch=x86_64"
   Then the exit code is 0
    And stdout is
    """
    expedition-1-1.fc29.x86_64
    └─ compass-1-1.fc29.x86_64
    """


# viewpoint Requires binoculars, which has x86_64 and i686 builds.
# Qualifying the root as viewpoint.x86_64 restricts only root selection; both
# binoculars architectures remain available as providers under the choice node.
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


# north-trail and south-trail are the only package names ending in "-trail".
# depth=0 isolates glob matching, and --arch excludes their source RPM roots.
Scenario: Package globs select every matching root
  Given I use repository "deptree"
   When I execute dnf with args "deptree --depth=0 --arch=x86_64 '*-trail'"
   Then the exit code is 0
    And stdout is
    """
    north-trail-1-1.fc29.x86_64

    south-trail-1-1.fc29.x86_64
    """


# Both waypost versions exist. The fully qualified version-1 NEVRA must select
# that single root even though version 2 is newer; depth=0 isolates this choice.
Scenario: Exact NEVRA selects an older package version
  Given I use repository "deptree"
   When I execute dnf with args "deptree --depth=0 waypost-1-1.fc29.x86_64"
   Then the exit code is 0
    And stdout is
    """
    waypost-1-1.fc29.x86_64
    """


# viewpoint Requires binoculars, which has x86_64 and i686 builds.
# Both provider architectures are allowed, while viewpoint.src is excluded from the roots.
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


# trailhead exists, but lost-trail does not. At depth=0, retain the matching root
# and report the missing specification separately on stderr.
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
