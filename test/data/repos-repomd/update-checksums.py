#!/usr/bin/python3


import hashlib
import os
import xml.etree.ElementTree


TOPDIR = os.path.dirname(os.path.abspath(__file__))

NAMESPACES = {
    "": "http://linux.duke.edu/metadata/repo",
    "rpm": "http://linux.duke.edu/metadata/rpm",
}


for key, value in NAMESPACES.items():
    xml.etree.ElementTree.register_namespace(key, value)


for repoid in os.listdir(TOPDIR):
    repo_dir = os.path.join(TOPDIR, repoid)

    if not os.path.isdir(repo_dir):
        continue

    print("Updating checksums in repo {}...".format(repoid))

    repomd_xml = os.path.join(repo_dir, "repodata", "repomd.xml")
    root = xml.etree.ElementTree.parse(repomd_xml)

    # iterate through all <data type="..."> nodes
    data_nodes = root.findall('./data', namespaces=NAMESPACES)
    for data in data_nodes:
        # determine indexed file location
        location = data.find(
            "./location", namespaces=NAMESPACES).attrib["href"]
        location = os.path.join(repo_dir, location)

        # compute and store the current checksum
        checksum = data.find("./checksum", namespaces=NAMESPACES)
        checksum_type = checksum.attrib["type"]
        hash = hashlib.new(checksum_type)
        hash.update(open(location, "rb").read())
        checksum.text = hash.hexdigest()

        # compute and store the current checksum of uncompressed file
        # it is equal to 'checksum' because we're not using any compression in order to keep data in plain text
        open_checksum = data.find("./open-checksum", namespaces=NAMESPACES)
        open_checksum_type = open_checksum.attrib["type"]
        open_hash = hashlib.new(checksum_type)
        open_hash.update(open(location, "rb").read())
        open_checksum.text = open_hash.hexdigest()

    root.write(repomd_xml)
