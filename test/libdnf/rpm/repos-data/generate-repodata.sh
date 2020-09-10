#!/bin/bash

createrepo_c --no-database --simple-md-filenames --revision=1550000000 dnf-ci-fedora
createrepo_c --no-database --simple-md-filenames --revision=1550000000 --baseurl http://dummy.com package-test-baseurl
