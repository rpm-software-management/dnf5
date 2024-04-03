import dbus
import os

DNFDAEMON_BUS_NAME = 'org.rpm.dnf.v0'
DNFDAEMON_OBJECT_PATH = '/' + DNFDAEMON_BUS_NAME.replace('.', '/')

IFACE_SESSION_MANAGER = '{}.SessionManager'.format(DNFDAEMON_BUS_NAME)
IFACE_REPO = '{}.rpm.Repo'.format(DNFDAEMON_BUS_NAME)
IFACE_REPOCONF = '{}.rpm.RepoConf'.format(DNFDAEMON_BUS_NAME)
IFACE_RPM = '{}.rpm.Rpm'.format(DNFDAEMON_BUS_NAME)
IFACE_GOAL = '{}.Goal'.format(DNFDAEMON_BUS_NAME)
IFACE_ADVISORY = '{}.Advisory'.format(DNFDAEMON_BUS_NAME)


bus = dbus.SystemBus()
iface_session = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, DNFDAEMON_OBJECT_PATH),
    dbus_interface=IFACE_SESSION_MANAGER)

session = iface_session.open_session(
    dbus.Dictionary({}, signature=dbus.Signature('sv')))

iface_rpm = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, session),
    dbus_interface=IFACE_RPM)
iface_advisory = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, session),
    dbus_interface=IFACE_ADVISORY)

# Information about the severity of the upgrade is stored in the updateinfo
# repository metadata and is not directly accessible from the list of possible
# upgrades. This means we first need to retrieve all advisories and then search
# them for severity for all potential upgrade packages.

# First get all available advisories, we are interested only in "severity" and
# "collections" (list of packages and modules) fields.
options = {
    "advisory_attrs": [
        # "advisoryid",
        # "name",
        # "title",
        # "type",
        "severity",
        # "status",
        # "vendor",
        # "description",
        # "buildtime",
        # "message",
        # "rights",
        "collections",
        # "references",
    ],
    "availability": "available",
}
advisory_list = iface_advisory.list(options)

# auxiliary dictionary to map package NEVRA to severity of advisory it belongs to
nevra_to_severity = dict()
for adv in advisory_list:
    severity = str(adv["severity"])
    for col in adv["collections"]:
        if "packages" in col:
            for pkg in col["packages"]:
                nevra = pkg["nevra"]
                nevra_to_severity[nevra] = severity


# retrieve potential upgrades and print the packages along with their
# respective severities
options = {
    "package_attrs": [
        "nevra",
        "repo_id",
    ],
    "scope": "upgrades",
    "latest-limit": 1,
}
upgrades = iface_rpm.list(options)
for pkg in upgrades:
    nevra = pkg["nevra"]
    severity = nevra_to_severity.get(nevra, "<unknown>")
    print("{} (@{}): {}".format(nevra, pkg["repo_id"], severity))
