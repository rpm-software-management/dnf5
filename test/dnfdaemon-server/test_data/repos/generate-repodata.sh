#!/bin/bash

REPOS=('repo-a' 'repo-b')

for d in ${REPOS[@]}; do
    mkdir ${d}
done

./fakerpm.sh pkg-a repo-a
./fakerpm.sh another-package repo-b

for d in ${REPOS[@]}; do
    createrepo_c --no-database --simple-md-filenames --revision=1550000000 ${d}
done
