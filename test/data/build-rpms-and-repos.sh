#!/bin/sh


usage() {
    echo "usage: $(basename $0) <binary-test-data-directory>"
}


if [ $# -ne 1 ]; then
    usage
    exit 1
fi


SCRIPT_DIRECTORY=$(dirname "$(readlink -f "$0")")
TARGET_DIRECTORY="$1"


case "${TARGET_DIRECTORY}" in
    "-h" | "--help")
        usage
        exit 0
        ;;
esac


build_rpms() {
    SPEC="$1"
    TARGET_DIR="$2"
    # use a temporary topdir to avoid writing to user's rpmbuild directory
    RPMBUILD_TOPDIR=$(mktemp -d)

    # Note: _build_name_fmt requires escaped %% for use in headerSprintf()
    rpmbuild -ba \
        --define="_topdir ${RPMBUILD_TOPDIR}" \
        --define="_srcrpmdir ${TARGET_DIR}" \
        --define="_rpmdir ${TARGET_DIR}" \
        --define="_build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm" \
        --define="_source_payload w1.gzdio" \
        --define="_binary_payload w1.gzdio" \
        "${SPEC}"

    rm -rf "${RPMBUILD_TOPDIR}"
}



# build cmdline-rpms/*.rpm
for SPEC in "${SCRIPT_DIRECTORY}"/cmdline-rpms/*.spec; do
    TARGET_DIR="${TARGET_DIRECTORY}/cmdline-rpms"
    build_rpms "${SPEC}" "${TARGET_DIR}"
done


# build repos-rpm/<repoid>/*.rpm
# and create repodata for each repos-rpm/<repoid>
for REPO_DIR in "${SCRIPT_DIRECTORY}"/repos-rpm/*; do
    REPOID=$(basename "${REPO_DIR}")
    TARGET_DIR="${TARGET_DIRECTORY}/repos-rpm/${REPOID}"
    for SPEC in "${REPO_DIR}"/*.spec; do
        build_rpms "${SPEC}" "${TARGET_DIR}"
    done
    createrepo_c "${TARGET_DIR}"
done
