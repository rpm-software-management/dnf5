#!/bin/bash
set -e

DIR="$(dirname "$(readlink -f "$0")")"
REPODIR="${DIR}/../repos"

# broken-package in dnf-ci-gpg must have incorrect checksum (appending at the end is enough)
echo "broken-package" >> "${REPODIR}/dnf-ci-gpg/noarch/broken-package-0.2.4-1.fc29.noarch.rpm"
echo "broken-package" >> "${REPODIR}/dnf-ci-gpg-updates/x86_64/wget-2.0.0-1.fc29.x86_64.rpm"
