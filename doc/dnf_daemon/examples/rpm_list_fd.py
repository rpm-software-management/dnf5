#!/usr/bin/python3
# Copyright Contributors to the libdnf project.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

"""
This is an example how to use `list_fd()` method of dnf5daemon-server Rpm interface.
"""

import dbus
import json
import os
import select
import time

DNFDAEMON_BUS_NAME = 'org.rpm.dnf.v0'
DNFDAEMON_OBJECT_PATH = '/' + DNFDAEMON_BUS_NAME.replace('.', '/')

IFACE_SESSION_MANAGER = '{}.SessionManager'.format(DNFDAEMON_BUS_NAME)
IFACE_RPM = '{}.rpm.Rpm'.format(DNFDAEMON_BUS_NAME)


def repoquery(iface_rpm, options):
    """Generator function that yields packages as they arrive from the server."""

    # create a pipe and pass the write end to the server
    pipe_r, pipe_w = os.pipe()
    # transfer id serves as an identifier of the pipe transfer for a signal emitted
    # after server finish. This example does not use it.
    iface_rpm.list_fd(options, pipe_w, "the_transfer_id")
    # close the write end - otherwise poll cannot detect the end of transmission
    os.close(pipe_w)

    # decoder that will be used to parse incomming data
    parser = json.JSONDecoder()

    # prepare for polling
    poller = select.poll()
    poller.register(pipe_r, select.POLLIN)
    # wait for data 10 secs at most
    timeout = 10000
    # 64k is a typical size of a pipe
    buffer_size = 65536

    # remaining string to parse (can contain unfinished json from previous run)
    to_parse = ""
    # remaining raw data (i.e. data before UTF decoding)
    raw_data = b""
    while True:
        # wait for data
        polled_event = poller.poll(timeout)
        if not polled_event:
            print("Timeout reached.")
            break

        # we know there is only one fd registered in poller
        descriptor, event = polled_event[0]
        # read a chunk of data
        buffer = os.read(descriptor, buffer_size)
        if not buffer:
            # end of file
            break

        raw_data += buffer
        try:
            to_parse += raw_data.decode()
            # decode successful, clear remaining raw data
            raw_data = b""
        except UnicodeDecodeError:
            # Buffer size split data in the middle of multibyte UTF character.
            # Need to read another chunk of data.
            continue

        # parse JSON objects from the string
        while to_parse:
            try:
                # skip all chars till begin of next JSON objects (new lines mostly)
                json_obj_start = to_parse.find('{')
                obj, end = parser.raw_decode(to_parse[json_obj_start:])
                yield obj
                to_parse = to_parse[(json_obj_start+end):]
            except json.decoder.JSONDecodeError as e:
                # this is just example which assumes that every decode error
                # means the data are incomplete (buffer size split the json
                # object in the middle). So the handler does not do anything
                # just break the parsing cycle and continue polling.
                break

    # finally close read end of the pipe
    os.close(pipe_r)

    # non-empty raw_data here means the raw_data.decode() failed
    if raw_data:
        raise Exception("Failed to decode part of received data.")

    # non-empty to_parse here means there are some unfinished (or generally unparsable)
    # JSON objects in the stream.
    if to_parse:
        raise Exception("Failed to parse part of received data.")


bus = dbus.SystemBus()

# open a new session with dnf5daemon-server
iface_session = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, DNFDAEMON_OBJECT_PATH),
    dbus_interface=IFACE_SESSION_MANAGER)
session = iface_session.open_session(
    dbus.Dictionary({}, signature=dbus.Signature('sv')))

options = {
    # retrieve all package attributes
    "package_attrs": [
        "name",
        "epoch",
        "version",
        "release",
        "arch",
        "repo_id",
        "from_repo_id",
        "is_installed",
        "install_size",
        "download_size",
        "sourcerpm",
        "summary",
        "url",
        "license",
        "description",
        "files",
        "changelogs",
        "provides",
        "requires",
        "requires_pre",
        "conflicts",
        "obsoletes",
        "recommends",
        "suggests",
        "enhances",
        "supplements",
        "evr",
        "nevra",
        "full_nevra",
        "reason",
        "vendor",
    ],
    # take all packages into account (other supported scopes are installed, available,
    # upgrades, upgradable)
    "scope": "all",
    # get only packages with name starting with "a"
    "patterns": ["a*"],
    # return only the latest version for each name.arch
    "latest-limit": 1,
}
iface_rpm = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, session),
    dbus_interface=IFACE_RPM)


i = 1
for pkg in repoquery(iface_rpm, options):
    print(i, pkg["nevra"])
    i += 1
