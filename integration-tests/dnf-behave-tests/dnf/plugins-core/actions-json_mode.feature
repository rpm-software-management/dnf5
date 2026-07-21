@not.with_dnf=4
Feature: Tests for libdnf5 actions plugin - json communication mode


Background:
  Given I enable plugin "actions"

    And I configure dnf with
      | key            | value                                              |
      | countme        | false                                              |
      | pluginconfpath | {context.dnf.installroot}/etc/dnf/libdnf5-plugins  |

    And I create file "/etc/dnf/libdnf5-plugins/actions.conf" with
    """
    [main]
    enabled = 1
    """

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_test_utils.py" with
"""
import json
import os
import sys

sys.path.append('dnf/steps')

from lib.json import diff_json_pattern_values


def send_request(request: dict):
    print(json.dumps(request), flush=True)


def send_request_string(request: str):
    print(request, flush=True)
    return


def check_reply(descr: str, expected: dict):
    reply_line = sys.stdin.readline()
    with open(sys.argv[1], 'a') as f:
        print(descr, end="", file=f)
        if not reply_line:
            if expected:
                print(': Error: No reply', file=f)
            else:
                print(': OK', file=f)
            return
        reply = json.loads(reply_line)
        diff = diff_json_pattern_values('', expected, reply)
        if diff:
            print(': Error:', diff, file=f)
            #print(reply_line, file=f)
        else:
            print(': OK', file=f)
"""

    And I use repository "dnf-ci-fedora" with configuration
      | key     | value                            |
      | name    | DNF CI fedora base repository    |
      | enabled | 1                                |
    And I use repository "dnf-ci-fedora-updates" with configuration
      | key     | value                            |
      | name    | DNF CI fedora updates repository |
      | enabled | 0                                |
    And I use repository "dnf-ci-thirdparty" with configuration
      | key     | value                            |
      | name    | DNF CI thirdparty repo, test     |
      | enabled | 0                                |


Scenario: Test actions_attrs, variables, actions_variables, configuration options, repos options, defining new repository, logging
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    post_base_setup:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_post_base_setup.py {context.dnf.installroot}/actions.log
    repos_configured:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_repos_configured.py {context.dnf.installroot}/actions.log
    """

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_post_base_setup.py" with
"""
from json_test_utils import *

send_request({"op": "get", "domain": "actions_attrs", "args": {"key": "*"}})
check_reply(
    'Get the values of all action plugin attributes',
    {"op": "reply", "requested_op": "get", "domain": "actions_attrs", "status": "OK",
     "return": {
         "actions_attrs": [{"key": "pid", "value": "*"},
                           {"key": "version", "value": "1.*"}]}})

send_request({"op": "get", "domain": "actions_attrs", "args": {"key": "nonexist_attribute"}})
check_reply(
    'Requesting the value of a non-existent actions plugin attribute returns empty list',
    {"op": "reply", "requested_op": "get", "domain": "actions_attrs", "status": "OK",
     "return": {"actions_attrs": []}})


send_request({"op": "set", "domain": "vars", "args": {"name": "test_var1", "value": "value1"}})
check_reply(
    'Set new variable "test_var1"',
    {"op": "reply", "requested_op": "set", "domain": "vars", "status": "OK",
     "return": {"vars": [{"name": "test_var1", "value": "value1"}]}})

send_request({"op": "set", "domain": "vars", "args": {"name": "test_var2", "value": "value2"}})
check_reply(
    'Set new variable "test_var2"',
    {"op": "reply", "requested_op": "set", "domain": "vars", "status": "OK",
     "return": {"vars": [{"name": "test_var2", "value": "value2"}]}})

send_request({"op": "set", "domain": "vars", "args": {"name": "test_var1", "value": "new_value1"}})
check_reply(
    'Change value of variable "test_var1"',
    {"op": "reply", "requested_op": "set", "domain": "vars", "status": "OK",
     "return": {"vars": [{"name": "test_var1", "value": "new_value1"}]}})

send_request({"op": "get", "domain": "vars", "args": {"name": "test_var*"}})
check_reply(
    'Get the values of all variables with the name matching "test_var*"',
    {"op": "reply", "requested_op": "get", "domain": "vars", "status": "OK",
     "return": {
         "vars": [{"name": "test_var1", "value": "new_value1"},
                  {"name": "test_var2", "value": "value2"}]}})

send_request({"op": "set", "domain": "vars", "args": {"name": "test_var2"}})
check_reply(
    'Remove (unset) variable "test_var2"',
    {"op": "reply", "requested_op": "set", "domain": "vars", "status": "OK",
     "return": {"vars": [{"name": "test_var2"}]}})

send_request({"op": "get", "domain": "vars", "args": {"name": "test_var*"}})
check_reply(
    'Get the values of all variables with the name matching "test_var*"',
    {"op": "reply", "requested_op": "get", "domain": "vars", "status": "OK",
     "return": {"vars": [{"name": "test_var1", "value": "new_value1"}]}})

send_request({"op": "get", "domain": "vars", "args": {"name": "nonexist_var"}})
check_reply(
    'Requesting the value of a non-existent variable returns empty list',
    {"op": "reply", "requested_op": "get", "domain": "vars", "status": "OK",
     "return": {"vars": []}})


send_request({"op": "set", "domain": "actions_vars", "args": {"name": "test_actions_var1", "value": "value1"}})
check_reply(
    'Set new actions variable "test_actions_var1"',
    {"op": "reply", "requested_op": "set", "domain": "actions_vars", "status": "OK",
     "return": {"actions_vars": [{"name": "test_actions_var1", "value": "value1"}]}})

send_request({"op": "set", "domain": "actions_vars", "args": {"name": "test_actions_var2", "value": "value2"}})
check_reply(
    'Set new actions variable "test_actions_var2"',
    {"op": "reply", "requested_op": "set", "domain": "actions_vars", "status": "OK",
     "return": {"actions_vars": [{"name": "test_actions_var2", "value": "value2"}]}})

send_request({"op": "set", "domain": "actions_vars", "args": {"name": "test_actions_var1", "value": "new_value1"}})
check_reply(
    'Change value of actions variable "test_actions_var1"',
    {"op": "reply", "requested_op": "set", "domain": "actions_vars", "status": "OK",
     "return": {"actions_vars": [{"name": "test_actions_var1", "value": "new_value1"}]}})

send_request({"op": "get", "domain": "actions_vars", "args": {"name": "test_actions_var*"}})
check_reply(
    'Get the values of all actions variables with the name matching "test_actions_var*"',
    {"op": "reply", "requested_op": "get", "domain": "actions_vars", "status": "OK",
     "return": {
         "actions_vars": [{"name": "test_actions_var1", "value": "new_value1"},
                          {"name": "test_actions_var2", "value": "value2"}]}})

send_request({"op": "set", "domain": "actions_vars", "args": {"name": "test_actions_var2"}})
check_reply(
    'Remove (unset) actions variable "test_actions_var2"',
    {"op": "reply", "requested_op": "set", "domain": "actions_vars", "status": "OK",
     "return": {"actions_vars": [{"name": "test_actions_var2"}]}})

send_request({"op": "get", "domain": "actions_vars", "args": {"name": "test_actions_var*"}})
check_reply(
    'Get the values of all actions variables with the name matching "test_actions_var*"',
    {"op": "reply", "requested_op": "get", "domain": "actions_vars", "status": "OK",
     "return": {"actions_vars": [{"name": "test_actions_var1", "value": "new_value1"}]}})

send_request({"op": "get", "domain": "actions_vars", "args": {"name": "nonexist_var"}})
check_reply(
    'Requesting the value of a non-existent actions variable returns empty list',
    {"op": "reply", "requested_op": "get", "domain": "actions_vars", "status": "OK",
     "return": {"actions_vars": []}})


send_request({"op": "get", "domain": "conf", "args": {"key": "countme"}})
check_reply(
    'Get value of "countme" option',
    {"op": "reply", "requested_op": "get", "domain": "conf", "status": "OK",
     "return": {"keys_val": [{"key": "countme", "value": "0"}]}})

send_request({"op": "set", "domain": "conf", "args": {"key": "countme", "value": "1"}})
check_reply(
    'Enable "countme" option',
    {"op": "reply", "requested_op": "set", "domain": "conf", "status": "OK",
     "return": {"keys_val": [{"key": "countme", "value": "1"}]}})

send_request({"op": "get", "domain": "conf", "args": {"key": "countme"}})
check_reply(
    'Get value of "countme" option',
    {"op": "reply", "requested_op": "get", "domain": "conf", "status": "OK",
     "return": {"keys_val": [{"key": "countme", "value": "1"}]}})
"""

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_repos_configured.py" with
"""
from json_test_utils import *

send_request({"op": "get", "domain": "actions_vars", "args": {"name": "test_actions_var*"}})
check_reply(
    'Get the values of all actions variables with the name matching "test_actions_var*" in next hook',
    {"op": "reply", "requested_op": "get", "domain": "actions_vars", "status": "OK",
     "return": {"actions_vars": [{"name": "test_actions_var1", "value": "new_value1"}]}})


send_request({"op": "set", "domain": "conf", "args": {"key": "dnf-ci-fedora*.enabled", "value": "1"}})
check_reply(
    'Enable "dnf-ci-fedora*" repos',
    {"op": "reply", "requested_op": "set", "domain": "conf", "status": "OK",
     "return": {
         "keys_val": [{"key": "dnf-ci-fedora.enabled", "value": "1"},
                      {"key": "dnf-ci-fedora-updates.enabled", "value": "1"}]}})

send_request(
    {"op": "new", "domain": "repoconf",
     "args": {
         "keys_val": [{"key": "repo_id", "value": "test-repo"},
                      {"key": "name", "value": "Test repository"},
                      {"key": "enabled", "value": "false"},
                      {"key": "baseurl", "value": "https://xyz.com/rpm"}]}})
check_reply(
    'Define new repo',
    {"op": "reply", "requested_op": "new", "domain": "repoconf", "status": "OK",
     "return": {
         "keys_val": [{"key": "repo_id", "value": "test-repo"},
                      {"key": "name", "value": "Test repository"},
                      {"key": "enabled", "value": "0"},
                      {"key": "baseurl", "value": "https://xyz.com/rpm"}]}})

send_request({"op": "get", "domain": "conf", "args": {"key": "*.enabled"}})
check_reply(
    'Get "enable" for all repos',
    {"op": "reply", "requested_op": "get", "domain": "conf", "status": "OK",
     "return": {
         "keys_val": [{"key": "dnf-ci-fedora.enabled", "value": "1"},
                      {"key": "dnf-ci-fedora-updates.enabled", "value": "1"},
                      {"key": "dnf-ci-thirdparty.enabled", "value": "0"},
                      {"key": "test-repo.enabled", "value": "0"}]}})


send_request({"op": "log", "args": {"level": "TRACE", "message": "Test log message 1"}})
check_reply(
    'Write TRACE to LOG',
    {"op": "reply", "requested_op": "log", "domain": "log", "status": "OK"})

send_request({"op": "log", "args": {"level": "DEBUG", "message": "Test log message 2"}})
check_reply(
    'Write DEBUG to LOG',
    {"op": "reply", "requested_op": "log", "domain": "log", "status": "OK"})

send_request({"op": "log", "args": {"level": "INFO", "message": "Test log message 3"}})
check_reply(
    'Write INFO to LOG',
    {"op": "reply", "requested_op": "log", "domain": "log", "status": "OK"})

send_request({"op": "log", "args": {"level": "NOTICE", "message": "Test log message 4"}})
check_reply(
    'Write NOTICE to LOG',
    {"op": "reply", "requested_op": "log", "domain": "log", "status": "OK"})

send_request({"op": "log", "args": {"level": "WARNING", "message": "Test log message 5"}})
check_reply(
    'Write WARNING to LOG',
    {"op": "reply", "requested_op": "log", "domain": "log", "status": "OK"})

send_request({"op": "log", "args": {"level": "ERROR", "message": "Test log message 6"}})
check_reply(
    'Write ERROR to LOG',
    {"op": "reply", "requested_op": "log", "domain": "log", "status": "OK"})

send_request({"op": "log", "args": {"level": "CRITICAL", "message": "Test log message 7"}})
check_reply(
    'Write CRITICAL to LOG',
    {"op": "reply", "requested_op": "log", "domain": "log", "status": "OK"})

send_request({"op": "log", "args": {"level": "BAD_LEVEL", "message": "Test log INFO message"}})
check_reply(
    'Write BAD_LEVEL to LOG',
    {"op": "reply", "requested_op": "log", "domain": "log", "status": "ERROR",
     "message": "Unknown log level \"BAD_LEVEL\""})


send_request({"op": "bad_op", "domain": "conf", "args": {"key": "*.enabled"}})
check_reply(
    'Request an unknown op',
    {"op": "reply", "requested_op": "bad_op", "status": "ERROR",
     "message": "Unknown operation \"bad_op\""})

send_request({"op": "get", "domain": "bad_domain", "args": {"key": "*.enabled"}})
check_reply(
    'Request an unknown domain for \"get\" op',
    {"op": "reply", "requested_op": "get", "domain":"bad_domain", "status": "ERROR",
     "message": "Unknown domain \"bad_domain\" for operation \"get\""})

send_request({"op": "set", "domain": "bad_domain", "args": {"key": "*.enabled"}})
check_reply(
    'Request an unknown domain for \"set\" op',
    {"op": "reply", "requested_op": "set", "domain":"bad_domain", "status": "ERROR",
     "message": "Unknown domain \"bad_domain\" for operation \"set\""})

send_request({"op": "new", "domain": "bad_domain", "args": {"key": "*.enabled"}})
check_reply(
    'Request an unknown domain for \"new\" op',
    {"op": "reply", "requested_op": "new", "domain":"bad_domain", "status": "ERROR",
     "message": "Unknown domain \"bad_domain\" for operation \"new\""})
"""

   When I execute dnf with args "repo list --all"
   Then the exit code is 0
    And stdout is
      """
      repo id               repo name                          status
      dnf-ci-fedora         DNF CI fedora base repository     enabled
      dnf-ci-fedora-updates DNF CI fedora updates repository  enabled
      dnf-ci-thirdparty     DNF CI thirdparty repo, test     disabled
      test-repo             Test repository                  disabled
      """

    And file "/actions.log" contents is
    """
    Get the values of all action plugin attributes: OK
    Requesting the value of a non-existent actions plugin attribute returns empty list: OK
    Set new variable "test_var1": OK
    Set new variable "test_var2": OK
    Change value of variable "test_var1": OK
    Get the values of all variables with the name matching "test_var*": OK
    Remove (unset) variable "test_var2": OK
    Get the values of all variables with the name matching "test_var*": OK
    Requesting the value of a non-existent variable returns empty list: OK
    Set new actions variable "test_actions_var1": OK
    Set new actions variable "test_actions_var2": OK
    Change value of actions variable "test_actions_var1": OK
    Get the values of all actions variables with the name matching "test_actions_var*": OK
    Remove (unset) actions variable "test_actions_var2": OK
    Get the values of all actions variables with the name matching "test_actions_var*": OK
    Requesting the value of a non-existent actions variable returns empty list: OK
    Get value of "countme" option: OK
    Enable "countme" option: OK
    Get value of "countme" option: OK
    Get the values of all actions variables with the name matching "test_actions_var*" in next hook: OK
    Enable "dnf-ci-fedora*" repos: OK
    Define new repo: OK
    Get "enable" for all repos: OK
    Write TRACE to LOG: OK
    Write DEBUG to LOG: OK
    Write INFO to LOG: OK
    Write NOTICE to LOG: OK
    Write WARNING to LOG: OK
    Write ERROR to LOG: OK
    Write CRITICAL to LOG: OK
    Write BAD_LEVEL to LOG: OK
    Request an unknown op: OK
    Request an unknown domain for "get" op: OK
    Request an unknown domain for "set" op: OK
    Request an unknown domain for "new" op: OK
    """

    And file "/var/log/dnf5.log" contains lines
    """
    TRACE Actions plugin: .* Test log message 1
    DEBUG Actions plugin: .* Test log message 2
    INFO Actions plugin: .* Test log message 3
    NOTICE Actions plugin: .* Test log message 4
    WARNING Actions plugin: .* Test log message 5
    ERROR Actions plugin: .* Test log message 6
    CRITICAL Actions plugin: .* Test log message 7
    """


Scenario: Test get packages and get trans_packages
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    repos_configured:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_repos_configured.py {context.dnf.installroot}/actions.log
    repos_loaded:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_repos_loaded.py {context.dnf.installroot}/actions.log
    pre_transaction:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_pre_trans.py {context.dnf.installroot}/actions.log
    """

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_repos_configured.py" with
"""
from json_test_utils import *

send_request({"op": "set", "domain": "conf", "args": {"key": "dnf-ci-fedora-updates.enabled", "value": "1"}})
check_reply(
    'Enable dnf-ci-fedora-updates repo',
    {"op": "reply", "requested_op": "set", "domain": "conf", "status": "OK",
     "return": {
         "keys_val": [{"key": "dnf-ci-fedora-updates.enabled", "value": "1"}]}})
"""

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_repos_loaded.py" with
"""
from json_test_utils import *

send_request(
    {"op": "get", "domain": "packages",
     "args": {
         "filters": [{"key": "name", "value": "f*", "operator": "GLOB"},
                     {"key": "version", "value": "1.3.3", "operator": "GTE"}],
         "output": ["name", "version", "arch", "release", "repo_id"]}})
check_reply(
    'Get packages with name "f*" and version >= "1.3.3"',
    {"op": "reply", "requested_op": "get", "domain": "packages", "status": "OK", "return": {"packages": [
        {"name": "flac", "arch": "src", "version": "1.3.3", "release": "1.fc29", "repo_id": "dnf-ci-fedora-updates"},
        {"name": "flac", "arch": "x86_64", "version": "1.3.3", "release": "1.fc29", "repo_id": "dnf-ci-fedora-updates"},
        {"name": "flac", "arch": "src", "version": "1.3.3", "release": "2.fc29", "repo_id": "dnf-ci-fedora-updates"},
        {"name": "flac", "arch": "x86_64", "version": "1.3.3", "release": "2.fc29", "repo_id": "dnf-ci-fedora-updates"},
        {"name": "flac", "arch": "src", "version": "1.3.3", "release": "3.fc29", "repo_id": "dnf-ci-fedora-updates"},
        {"name": "flac", "arch": "x86_64", "version": "1.3.3", "release": "3.fc29", "repo_id": "dnf-ci-fedora-updates"},
        {"name": "flac-libs", "arch": "x86_64", "version": "1.3.3", "release": "1.fc29",
         "repo_id": "dnf-ci-fedora-updates"},
        {"name": "flac-libs", "arch": "x86_64", "version": "1.3.3", "release": "2.fc29",
         "repo_id": "dnf-ci-fedora-updates"},
        {"name": "flac-libs", "arch": "x86_64", "version": "1.3.3", "release": "3.fc29",
         "repo_id": "dnf-ci-fedora-updates"},
        {"name": "fedora-release", "arch": "noarch", "version": "29", "release": "1", "repo_id": "dnf-ci-fedora"},
        {"name": "fedora-release", "arch": "src", "version": "29", "release": "1", "repo_id": "dnf-ci-fedora"},
        {"name": "filesystem", "arch": "src", "version": "3.9", "release": "2.fc29", "repo_id": "dnf-ci-fedora"},
        {"name": "filesystem", "arch": "x86_64", "version": "3.9", "release": "2.fc29", "repo_id": "dnf-ci-fedora"},
        {"name": "filesystem-content", "arch": "x86_64", "version": "3.9", "release": "2.fc29",
         "repo_id": "dnf-ci-fedora"}]}})

send_request(
    {"op": "get", "domain": "packages",
     "args": {
         "filters": [{"key": "name", "value": "lame*", "operator": "GLOB"}],
         "output": ["nevra"]}})
check_reply(
    'Get packages with name "lame*"',
    {"op": "reply", "requested_op": "get", "domain": "packages", "status": "OK", "return": {"packages": [
        {"nevra": "lame-libs-3.100-5.fc29.x86_64"}, {"nevra": "lame-libs-3.100-4.fc29.x86_64"}]}})

send_request(
    {"op": "get", "domain": "packages",
     "args": {
         "params": [{"key": "IGNORE_EXCLUDES"}],
         "filters": [{"key": "name", "value": "lame*", "operator": "GLOB"}],
         "output": ["nevra"]}})
check_reply(
    'Get packages with name "lame*", ignore excludes',
    {"op": "reply", "requested_op": "get", "domain": "packages", "status": "OK", "return": {"packages": [
        { "nevra": "lame-3.100-5.fc29.src" }, { "nevra": "lame-3.100-5.fc29.x86_64" },
        { "nevra": "lame-libs-3.100-5.fc29.x86_64" }, { "nevra": "lame-3.100-4.fc29.src" },
        { "nevra": "lame-3.100-4.fc29.x86_64" }, { "nevra": "lame-libs-3.100-4.fc29.x86_64"}]}})

send_request(
    {"op": "get", "domain": "packages",
     "args": {
         "params": [{"key": "UNKNOWN"}],
         "filters": [{"key": "name", "value": "lame*", "operator": "GLOB"}],
         "output": ["nevra"]}})
check_reply(
    'Get packages with name "lame*", use unknown key in params',
    {"op": "reply", "requested_op": "get", "domain": "packages", "status": "ERROR",
     "message": "Bad key \"UNKNOWN\" for params"})

send_request(
    {"op": "get", "domain": "packages",
     "args": {
         "filters": [{"key": "direction", "value": "IN"}],
         "output": ["nevra", "direction"]}})
check_reply(
    'Get packages, use "direction" output outside "trans_packages" domain',
    {"op": "reply", "requested_op": "get", "domain": "packages", "status": "ERROR",
     "message": "Requested output \"direction\" outside the domain \"trans_packages\""})

send_request(
    {"op": "get", "domain": "packages",
     "args": {
         "filters": [{"key": "direction", "value": "IN"}],
         "output": ["nevra", "action"]}})
check_reply(
    'Get packages, use "action" output outside "trans_packages" domain',
    {"op": "reply", "requested_op": "get", "domain": "packages", "status": "ERROR",
     "message": "Requested output \"action\" outside the domain \"trans_packages\""})

send_request(
    {"op": "get", "domain": "packages",
     "args": {
         "filters": [{"key": "direction", "value": "IN"}],
         "output": ["name"]}})
check_reply(
    'Get packages, use "direction" filter outside "trans_packages" domain',
    {"op": "reply", "requested_op": "get", "domain": "packages", "status": "ERROR",
     "message": "Used \"direction\" filter outside the \"trans_packages\" domain"})

send_request(
    {"op": "get", "domain": "trans_packages",
     "args": {
         "filters": [{"key": "direction", "value": "IN"}],
         "output": ["nevra"]}})
check_reply(
    'Get trans_packages outside "goal_resolved", "pre_transaction" and "post_transaction" hooks',
    {"op": "reply", "requested_op": "get", "domain": "trans_packages", "status": "ERROR",
     "message": "Domain \"trans_packages\" used outside the hooks \"goal_resolved\", \"pre_transaction\" and \"post_transaction\""})
"""

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_pre_trans.py" with
"""
from json_test_utils import *

send_request(
    {"op": "get", "domain": "trans_packages",
     "args": {
         "filters": [{"key": "direction", "value": "IN"}, {"key": "arch", "value": "x86_64"}],
         "output": ["action", "name", "version", "repo_id"]}})
check_reply(
    'Get "IN" trans_packages with arch == "x86_64"',
    {"op": "reply", "requested_op": "get", "domain": "trans_packages", "status": "OK",
     "return": {
         "trans_packages": [
             {"action": "I", "name": "glibc", "version": "2.28", "repo_id": "dnf-ci-fedora-updates"},
             {"action": "I", "name": "glibc-all-langpacks", "version": "2.28", "repo_id": "dnf-ci-fedora-updates"},
             {"action": "I", "name": "glibc-common", "version": "2.28", "repo_id": "dnf-ci-fedora-updates"},
             {"action": "I", "name": "filesystem", "version": "3.9", "repo_id": "dnf-ci-fedora"}]}})

send_request(
    {"op": "get", "domain": "trans_packages",
     "args": {
         "filters": [{"key": "direction", "value": "OUT"}],
         "output": ["action", "name", "version", "repo_id"]}})
check_reply(
    'Get "OUT" trans_packages',
    {"op": "reply", "requested_op": "get", "domain": "trans_packages", "status": "OK",
     "return": {"trans_packages": []}})

send_request(
    {"op": "get", "domain": "trans_packages",
     "args": {
         "filters": [{"key": "direction", "value": "UNKNOWN"}],
         "output": ["action", "name", "version", "repo_id"]}})
check_reply(
    'Get trans_packages, unknown value for direction filter',
    {"op": "reply", "requested_op": "get", "domain": "trans_packages", "status": "ERROR",
     "message": "Bad \"UNKNOWN\" value for \"direction\" filter"})

send_request(
    {"op": "get", "domain": "trans_packages",
     "args": {
         "filters": [
             {"key": "name", "value": "glibc"}, {"key": "arch", "value": "x86_64"}, {"key": "version", "value": "2.28"},
             {"key": "release", "value": "26.fc29"}, {"key": "epoch", "value": "0"},
             {"key": "nevra", "value": "glibc-0:2.28-26.fc29.x86_64"}, {"key": "repo_id", "value": "dnf-ci-fedora-updates"}],
         "output": ["action", "name", "version", "repo_id"]}})
check_reply(
    'Get trans_packages, test filters',
    {"op": "reply", "requested_op": "get", "domain": "trans_packages", "status": "OK",
     "return": {
         "trans_packages": [
             {"action": "I", "name": "glibc", "version": "2.28", "repo_id": "dnf-ci-fedora-updates"}]}})

send_request(
    {"op": "get", "domain": "trans_packages",
     "args": {
         "filters": [{"key": "name", "value": "glibc"}],
         "output": [
             "direction", "action", "name", "arch", "unknown", "version", "release", "epoch", "na", "evr", "nevra",
             "full_nevra", "download_size", "install_size", "repo_id", "license", "location", "vendor"]}})
check_reply(
    'Get attributes of "glibc" trans_package, unknown attribute is ignored',
    {"op": "reply", "requested_op": "get", "domain": "trans_packages", "status": "OK",
     "return": {
         "trans_packages": [
             {"direction":"IN", "action": "I", "name": "glibc", "arch":"x86_64", "version": "2.28",
              "release":"26.fc29", "epoch":"0", "na":"glibc.x86_64", "evr":"2.28-26.fc29",
              "nevra":"glibc-2.28-26.fc29.x86_64", "full_nevra":"glibc-0:2.28-26.fc29.x86_64",
              "download_size":"*", "install_size":"*", "repo_id":"dnf-ci-fedora-updates",
              "license":"LGPLv2+ and LGPLv2+ with exceptions and GPLv2+ and GPLv2+ with exceptions and BSD and Inner-Net and ISC and Public Domain and GFDL",
              "location":"x86_64/glibc-2.28-26.fc29.x86_64.rpm", "vendor":""}]}})

send_request(
    {"op": "get", "domain": "trans_packages",
     "args": {
         "filters": [{"key": "unknown_key", "value": "x86_64"}],
         "output": ["action", "name", "version", "repo_id"]}})
check_reply(
    'Get trans_packages, use unknown key in filters',
    {"op": "reply", "requested_op": "get", "domain": "trans_packages", "status": "ERROR",
     "message": "Unknown package filter key \"unknown_key\""})

send_request(
    {"op": "get", "domain": "trans_packages",
     "args": {
         "filters": [{"key": "name", "value": "glibc", "operator":"UNKNOWN_OPERATOR"}],
         "output": ["action", "name", "version", "repo_id"]}})
check_reply(
    'Get trans_packages, use unknown compare operator in filters',
    {"op": "reply", "requested_op": "get", "domain": "trans_packages", "status": "ERROR",
     "message": "Bad compare operator \"UNKNOWN_OPERATOR\""})
"""

   When I execute dnf with args "--exclude=lame install glibc"
   Then the exit code is 0
    And Transaction is following
       | Action        | Package                                   |
       | install       | glibc-0:2.28-26.fc29.x86_64               |
       | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
       | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
       | install-dep   | glibc-common-0:2.28-26.fc29.x86_64        |
       | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
       | install-dep   | basesystem-0:11-6.fc29.noarch             |

    And file "/actions.log" contents is
    """
    Enable dnf-ci-fedora-updates repo: OK
    Get packages with name "f*" and version >= "1.3.3": OK
    Get packages with name "lame*": OK
    Get packages with name "lame*", ignore excludes: OK
    Get packages with name "lame*", use unknown key in params: OK
    Get packages, use "direction" output outside "trans_packages" domain: OK
    Get packages, use "action" output outside "trans_packages" domain: OK
    Get packages, use "direction" filter outside "trans_packages" domain: OK
    Get trans_packages outside "goal_resolved", "pre_transaction" and "post_transaction" hooks: OK
    Get "IN" trans_packages with arch == "x86_64": OK
    Get "OUT" trans_packages: OK
    Get trans_packages, unknown value for direction filter: OK
    Get trans_packages, test filters: OK
    Get attributes of "glibc" trans_package, unknown attribute is ignored: OK
    Get trans_packages, use unknown key in filters: OK
    Get trans_packages, use unknown compare operator in filters: OK
    """


Scenario: Test get paths of command line packages
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    pre_add_cmdline_packages:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_pre_add_cmdline_packages.py {context.dnf.installroot}/actions.log
    """

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_pre_add_cmdline_packages.py" with
"""
from json_test_utils import *

send_request(
    {"op": "get", "domain": "cmdline_packages_paths",
     "args": {"filters": [{"key": "path", "value": "/local2/*", "operator": "GLOB"}]}})
check_reply(
    'Get paths of all cmdline packages with the path matching "/local2/*"',
    {"op": "reply", "requested_op": "get", "domain": "cmdline_packages_paths", "status": "OK",
     "return": {"cmdline_packages_paths": ["/local2/packageB.rpm", "/local2/packageC.rpm"]}})
"""

   When I execute dnf with args "repoquery /local1/packageA.rpm /local2/packageB.rpm /local2/packageC.rpm"
   Then the exit code is 1

    And file "/actions.log" contents is
    """
    Get paths of all cmdline packages with the path matching "/local2/*": OK
    """


Scenario: Test syntax errors (missing attributes, incorrect json, truncated - incomplete input) in json requests
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    pre_base_setup:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_pre_base_setup.py {context.dnf.installroot}/actions.log
    post_base_setup:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_post_base_setup.py {context.dnf.installroot}/actions.log
    repos_configured:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_repos_configured.py {context.dnf.installroot}/actions.log
    """

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_pre_base_setup.py" with
"""
from json_test_utils import *

send_request({"domain":"vars", "args": {"name": "test_var1", "value": "value1"}})
check_reply(
    'Missing "op"',
    {"op": "reply", "status": "ERROR",
     "message": "Key \"op\" not found"})

send_request({"op": "set", "args": {"name": "test_var1", "value": "value1"}})
check_reply(
    'Missing "domain" for "set" op',
    {"op": "reply", "requested_op": "set", "status": "ERROR",
     "message": "Key \"domain\" not found"})

# Send json structure with missing starting '{'
send_request_string('"op": "set", "args": {"name": "test_var1", "value": "val')
check_reply("Missing starting '{' char", {})
"""

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_post_base_setup.py" with
"""
from json_test_utils import *

# Send invalid json structure
send_request_string('{"op": "set", "", "args": {"name": "test_var1", "value": "val')
check_reply("Invalid json structure", {})
"""

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_repos_configured.py" with
"""
from json_test_utils import *

# Send incomplete json structure and close output
send_request_string('{"op": "set", "args": {"name": "test_var1", "value": "val')
os.close(1)
check_reply("Incomplete input", {})
"""

   When I execute dnf with args "repoquery dwm"
   Then the exit code is 0

    And file "/actions.log" contents is
    """
    Missing "op": OK
    Missing "domain" for "set" op: OK
    Missing starting '{' char: OK
    Invalid json structure: OK
    Incomplete input: OK
    """

    And file "/var/log/dnf5.log" contains lines
    """
    Syntax error in json request .* Missing starting '{' char
    Syntax error in json request .* expected
    Syntax error in json request .* Incomplete input
    """


Scenario: Testing the "error" action message (JSON mode)
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    repos_configured:::mode=json:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_error1.py {context.dnf.installroot}/actions.log
    repos_configured:::mode=json raise_error=0:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_error2.py {context.dnf.installroot}/actions.log
    repos_configured:::mode=json raise_error=1:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_error3.py {context.dnf.installroot}/actions.log
    """

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_error1.py" with
"""
from json_test_utils import *

send_request({"op": "error", "args": {"message": "Error in action process 1"}})
check_reply(
    'Sent "error" message, no "raise_error" defined',
    {"op": "reply", "requested_op": "error", "domain": "error", "status": "OK"})
"""

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_error2.py" with
"""
from json_test_utils import *

send_request({"op": "error", "args": {"message": "Error in action process 2"}})
check_reply(
    'Sent "error" message, "raise_error=0"',
    {"op": "reply", "requested_op": "error", "domain": "error", "status": "OK"})
"""

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_error3.py" with
"""
from json_test_utils import *

send_request({"op": "error", "args": {"message": "Error in action process 3"}})
check_reply(
    'Sent "error" message, "raise_error=1"',
    {"op": "reply", "requested_op": "error", "domain": "error", "status": "OK"})
"""

   When I execute dnf with args "repo list"
   Then the exit code is 1

    And file "/actions.log" contents is
    """
    Sent "error" message, no "raise_error" defined: OK
    Sent "error" message, "raise_error=0": OK
    Sent "error" message, "raise_error=1": Error: No reply
    """

    And stderr contains "Action sent error message: Error in action process 3"

    And file "/var/log/dnf5.log" contains lines
    """
    ERROR Actions plugin: .* Action sent error message: Error in action process 1
    ERROR Actions plugin: .* Action sent error message: Error in action process 2
    """


Scenario: Testing the "stop" action request (JSON mode)
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    repos_configured:::mode=json raise_error=1:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_stop.py {context.dnf.installroot}/actions.log
    """

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_stop.py" with
"""
from json_test_utils import *

send_request({"op": "stop", "args": {"message": "I want to stop the task"}})
check_reply(
    'Sent "stop" request',
    {"op": "reply", "requested_op": "stop", "domain": "stop", "status": "OK"})
"""

   When I execute dnf with args "repo list"
   Then the exit code is 1

    And file "/actions.log" contents is
    """
    Sent "stop" request: Error: No reply
    """

    And stderr contains "Action calls for stop: I want to stop the task"


Scenario: Testing the "stop" action request (JSON mode), must thrown exception even with "raise_error=0"
  Given I create and substitute file "/etc/dnf/libdnf5-plugins/actions.d/test.actions" with
    """
    repos_configured:::mode=json raise_error=0:/bin/python3 {context.dnf.installroot}/etc/dnf/libdnf5-plugins/actions.d/json_hook_stop.py {context.dnf.installroot}/actions.log
    """

    And I create file "/etc/dnf/libdnf5-plugins/actions.d/json_hook_stop.py" with
"""
from json_test_utils import *

send_request({"op": "stop", "args": {"message": "I want to stop the task"}})
check_reply(
    'Sent "stop" request',
    {"op": "reply", "requested_op": "stop", "domain": "stop", "status": "OK"})
"""

   When I execute dnf with args "repo list"
   Then the exit code is 1

    And file "/actions.log" contents is
    """
    Sent "stop" request: Error: No reply
    """

    And stderr contains "Action calls for stop: I want to stop the task"
