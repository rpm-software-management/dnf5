#!/bin/sh

dnf reposync --repoid=fedora --repoid=fedora-debuginfo --repoid=fedora-source -x '*' --destdir . --download-metadata

# metalink doesn't point to the right location
# edit /etc/yum.repos.d/fedora-updates.repo and use baseurl instead of metalink
dnf reposync --repoid=updates --repoid=updates-debuginfo --repoid=updates-source -x '*' --destdir . --download-metadata

dnf reposync --repoid=updates-testing --repoid=updates-testing-debuginfo --repoid=updates-testing-source -x '*' --destdir . --download-metadata

dnf reposync --repoid=fedora-modular --repoid=fedora-modular-debuginfo --repoid=fedora-modular-source -x '*' --destdir . --download-metadata

dnf reposync --repoid=updates-modular --repoid=updates-modular-debuginfo --repoid=updates-modular-source -x '*' --destdir . --download-metadata
