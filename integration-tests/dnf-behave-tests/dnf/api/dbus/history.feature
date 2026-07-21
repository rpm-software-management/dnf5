@not.with_mode=dnf5
@dnf5daemon
Feature: D-Bus api: history

Background: Prepare transactions history
Given I use repository "history-recent"
  And I use repository "simple-base"
  And I successfully execute dnf with args "install fragola-1"
  And I successfully execute dnf with args "upgrade fragola-2"
  And I successfully execute dnf with args "install labirinto"
  And I successfully execute dnf with args "upgrade fragola"
  And I successfully execute dnf with args "downgrade fragola"
  And I successfully execute dnf with args "remove labirinto"
  # set known times for transactions in the history database
  And I adjust history database with query
  """
  UPDATE trans SET dt_begin=id*10, dt_end=id*10
  """
 Then the exit code is 0


Scenario Outline: History::recent_changes() returns changeset according to options
 When I execute python libdnf5 dbus api script with history interface
    """
    options = {{
        "upgraded_packages" : <upgraded>,
        "installed_packages" : <installed>,
        "downgraded_packages" : <downgraded>,
        "removed_packages" : <removed>,
    }}
    changeset = iface_history.recent_changes(options)
    print(sorted([str(k) for k in changeset.keys()]))
    """
 Then stdout is
    """
    <output>
    """

Examples:
| upgraded  | installed | downgraded    | removed   | output |
| True      | False     | False         | False     | ['upgraded'] |
| True      | True      | False         | False     | ['installed', 'upgraded'] |
| True      | True      | True          | False     | ['downgraded', 'installed', 'upgraded'] |
| True      | True      | True          | True      | ['downgraded', 'installed', 'removed', 'upgraded'] |


Scenario: History::recent_changes() uses the latest transaction if "since" option is not specified
 When I execute python libdnf5 dbus api script with history interface
    """
    options = {{
    }}

    changeset = iface_history.recent_changes(options)
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 0
    downgraded: 0
    removed: 1
    NEVRA: labirinto-1.0-1.fc29.x86_64
    Transaction time: 60
    """

Scenario: History::recent_changes() returns changes since a point in time
 # Set the `since` parameter before/after each transaction present in the history db.
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes({{"since": dbus.Int64(9)}})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 1
    NEVRA: fragola-3-1.noarch
    Summary: Made up package
    Transaction time: 50
    upgraded: 0
    downgraded: 0
    removed: 0
    """
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes({{"since": dbus.Int64(19)}})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 1
    NEVRA: fragola-3-1.noarch
    Summary: Made up package
    Original EVR: 1-1
    Transaction time: 50
    Advisories: DUMMY-3
    downgraded: 0
    removed: 0
    """
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes({{"since": dbus.Int64(29)}})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 1
    NEVRA: fragola-3-1.noarch
    Summary: Made up package
    Original EVR: 2-1
    Transaction time: 50
    Advisories: DUMMY-3
    downgraded: 0
    removed: 0
    """
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes({{"since": dbus.Int64(39)}})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 1
    NEVRA: fragola-3-1.noarch
    Summary: Made up package
    Original EVR: 2-1
    Transaction time: 50
    Advisories: DUMMY-3
    downgraded: 0
    removed: 1
    NEVRA: labirinto-1.0-1.fc29.x86_64
    Transaction time: 60
    """
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes({{"since": dbus.Int64(49)}})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 0
    downgraded: 1
    NEVRA: fragola-3-1.noarch
    Summary: Made up package
    Original EVR: 4-1
    Transaction time: 50
    removed: 1
    NEVRA: labirinto-1.0-1.fc29.x86_64
    Transaction time: 60
    """
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes({{"since": dbus.Int64(59)}})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 0
    downgraded: 0
    removed: 1
    NEVRA: labirinto-1.0-1.fc29.x86_64
    Transaction time: 60
    """
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes({{"since": dbus.Int64(69)}})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 0
    downgraded: 0
    removed: 0
    """

Scenario: History::recent_changes() accepts "include_advisory" option
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes(
        {{
        "since": dbus.Int64(19),
        "include_advisory": True,
        }})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 1
    NEVRA: fragola-3-1.noarch
    Summary: Made up package
    Original EVR: 1-1
    Transaction time: 50
    Advisories: DUMMY-3
    downgraded: 0
    removed: 0
    """
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes(
        {{
        "since": dbus.Int64(19),
        "include_advisory": False,
        }})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 1
    NEVRA: fragola-3-1.noarch
    Summary: Made up package
    Original EVR: 1-1
    Transaction time: 50
    downgraded: 0
    removed: 0
    """

Scenario: History::recent_changes() accepts "all_advisories" option
 When I execute python libdnf5 dbus api script with history interface
    """
    changeset = iface_history.recent_changes(
        {{
        "since": dbus.Int64(19),
        "include_advisory": True,
        "all_advisories": True,
        }})
    print_recent_history(changeset)
    """
 Then the exit code is 0
 And stdout is
    """
    installed: 0
    upgraded: 1
    NEVRA: fragola-3-1.noarch
    Summary: Made up package
    Original EVR: 1-1
    Transaction time: 50
    Advisories: DUMMY-3, DUMMY-2
    downgraded: 0
    removed: 0
    """


Scenario: History::list() all transactions by default
 When I execute python libdnf5 dbus api script with history interface
    """
    options = {{
    }}

    transactions = dbus_to_python(iface_history.list(options))
    for k in transactions:
        print(k)
    """
 Then the exit code is 0
 And stdout is
    """
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 60, 'id': 6, 'installed': [], 'reinstalled': [], 'removed': [{{'arch': 'x86_64', 'evr': '1.0-1.fc29', 'name': 'labirinto'}}], 'start': 60, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [{{'arch': 'noarch', 'evr': '3-1', 'name': 'fragola', 'original_evr': '4-1'}}], 'end': 50, 'id': 5, 'installed': [], 'reinstalled': [], 'removed': [], 'start': 50, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 40, 'id': 4, 'installed': [], 'reinstalled': [], 'removed': [], 'start': 40, 'status': 'Ok', 'upgraded': [{{'arch': 'noarch', 'evr': '4-1', 'name': 'fragola', 'original_evr': '2-1'}}], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 30, 'id': 3, 'installed': [{{'arch': 'x86_64', 'evr': '1.0-1.fc29', 'name': 'labirinto'}}], 'reinstalled': [], 'removed': [], 'start': 30, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 20, 'id': 2, 'installed': [], 'reinstalled': [], 'removed': [], 'start': 20, 'status': 'Ok', 'upgraded': [{{'arch': 'noarch', 'evr': '2-1', 'name': 'fragola', 'original_evr': '1-1'}}], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 10, 'id': 1, 'installed': [{{'arch': 'noarch', 'evr': '1-1', 'name': 'fragola'}}], 'reinstalled': [], 'removed': [], 'start': 10, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    """


Scenario: History::list() reversed with a limit
 When I execute python libdnf5 dbus api script with history interface
    """
    options = {{
        "reverse": True,
        "limit": dbus.Int64(3)
    }}

    transactions = dbus_to_python(iface_history.list(options))
    for k in transactions:
        print(k)
    """
 Then the exit code is 0
  And stdout is
    """
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 10, 'id': 1, 'installed': [{{'arch': 'noarch', 'evr': '1-1', 'name': 'fragola'}}], 'reinstalled': [], 'removed': [], 'start': 10, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 20, 'id': 2, 'installed': [], 'reinstalled': [], 'removed': [], 'start': 20, 'status': 'Ok', 'upgraded': [{{'arch': 'noarch', 'evr': '2-1', 'name': 'fragola', 'original_evr': '1-1'}}], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 30, 'id': 3, 'installed': [{{'arch': 'x86_64', 'evr': '1.0-1.fc29', 'name': 'labirinto'}}], 'reinstalled': [], 'removed': [], 'start': 30, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    """


Scenario: History::list() with specified package
 When I execute python libdnf5 dbus api script with history interface
    """
    options = {{
        "contains_pkgs": ["fragola"],
    }}

    transactions = dbus_to_python(iface_history.list(options))
    for k in transactions:
        print(k)
    """
 Then the exit code is 0
  And stdout is
    """
    {{'description': 'dnf5daemon-server', 'downgraded': [{{'arch': 'noarch', 'evr': '3-1', 'name': 'fragola', 'original_evr': '4-1'}}], 'end': 50, 'id': 5, 'installed': [], 'reinstalled': [], 'removed': [], 'start': 50, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 40, 'id': 4, 'installed': [], 'reinstalled': [], 'removed': [], 'start': 40, 'status': 'Ok', 'upgraded': [{{'arch': 'noarch', 'evr': '4-1', 'name': 'fragola', 'original_evr': '2-1'}}], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 20, 'id': 2, 'installed': [], 'reinstalled': [], 'removed': [], 'start': 20, 'status': 'Ok', 'upgraded': [{{'arch': 'noarch', 'evr': '2-1', 'name': 'fragola', 'original_evr': '1-1'}}], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 10, 'id': 1, 'installed': [{{'arch': 'noarch', 'evr': '1-1', 'name': 'fragola'}}], 'reinstalled': [], 'removed': [], 'start': 10, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    """


Scenario: History::list() since timestamp
 When I execute python libdnf5 dbus api script with history interface
    """
    options = {{
        "since": dbus.Int64(40)
    }}

    transactions = dbus_to_python(iface_history.list(options))
    for k in transactions:
        print(k)
    """
 Then the exit code is 0
  And stdout is
    """
    {{'description': 'dnf5daemon-server', 'downgraded': [], 'end': 60, 'id': 6, 'installed': [], 'reinstalled': [], 'removed': [{{'arch': 'x86_64', 'evr': '1.0-1.fc29', 'name': 'labirinto'}}], 'start': 60, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    {{'description': 'dnf5daemon-server', 'downgraded': [{{'arch': 'noarch', 'evr': '3-1', 'name': 'fragola', 'original_evr': '4-1'}}], 'end': 50, 'id': 5, 'installed': [], 'reinstalled': [], 'removed': [], 'start': 50, 'status': 'Ok', 'upgraded': [], 'user_id': 0}}
    """


Scenario: History::list() without packages and limited transaction attributes
 When I execute python libdnf5 dbus api script with history interface
    """
    options = {{
        "transaction_attrs": ["id"],
        "include_packages": False
    }}

    transactions = dbus_to_python(iface_history.list(options))
    for k in transactions:
        print(k)
    """
 Then the exit code is 0
  And stdout is
    """
    {{'id': 6}}
    {{'id': 5}}
    {{'id': 4}}
    {{'id': 3}}
    {{'id': 2}}
    {{'id': 1}}
    """


Scenario: History::list() with limited package attributes and limited transaction attributes
 When I execute python libdnf5 dbus api script with history interface
    """
    options = {{
        "transaction_attrs": ["id"],
        "package_attrs": ["name"]
    }}

    transactions = dbus_to_python(iface_history.list(options))
    for k in transactions:
        print(k)
    """
 Then the exit code is 0
  And stdout is
    """
    {{'downgraded': [], 'id': 6, 'installed': [], 'reinstalled': [], 'removed': [{{'name': 'labirinto'}}], 'upgraded': []}}
    {{'downgraded': [{{'name': 'fragola', 'original_evr': '4-1'}}], 'id': 5, 'installed': [], 'reinstalled': [], 'removed': [], 'upgraded': []}}
    {{'downgraded': [], 'id': 4, 'installed': [], 'reinstalled': [], 'removed': [], 'upgraded': [{{'name': 'fragola', 'original_evr': '2-1'}}]}}
    {{'downgraded': [], 'id': 3, 'installed': [{{'name': 'labirinto'}}], 'reinstalled': [], 'removed': [], 'upgraded': []}}
    {{'downgraded': [], 'id': 2, 'installed': [], 'reinstalled': [], 'removed': [], 'upgraded': [{{'name': 'fragola', 'original_evr': '1-1'}}]}}
    {{'downgraded': [], 'id': 1, 'installed': [{{'name': 'fragola'}}], 'reinstalled': [], 'removed': [], 'upgraded': []}}
    """
