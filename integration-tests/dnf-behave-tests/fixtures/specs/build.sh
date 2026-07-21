#!/bin/bash


export LC_ALL=C

set -e

DIR="$(dirname "$(readlink -f "$0")")"
ARCH="x86_64"
DIST=".fc29"
REPODIR="$DIR/../repos"
GPGDIR="$DIR/../gpgkeys"
CERTSDIR="$DIR/../certificates"
GROUPS_FILENAME="comps.xml"
UPDATEINFO_FILENAME="updateinfo.xml"
MODULARITY="true"
MODULES_FILENAME="modules.yaml"
FORCE_REBUILD=

fatal()
{
    printf >&2 "Error: %s\n" "$*"
    exit 1
}

# Modularity is disabled since RHEL 11.
# Detect OS with behave code because it implements workarounds.
want_modularity()
{
    PYTHONPATH="$DIR/../.." python3 - <<'EOF'
from common.lib.os_version import want_modularity
if want_modularity():
   print('true')
else:
   print('false')
EOF
}
MODULARITY=$(want_modularity)

while [ "$1" != "" ]; do
    case "$1" in
        -f|--force-rebuild) FORCE_REBUILD="true"; shift;;
        *) fatal "Non-implemented option: $1"
    esac
done

if [ "$FORCE_REBUILD" = true ]; then
    # remove all generated content
    find "$DIR" -name "*.sha256" -delete
    rm -rf "$REPODIR"
fi

mkdir -p "$REPODIR"

for path in "$DIR"/*/*.spec; do
    REPO="$(basename "$(dirname "$path")")"
    SPEC_NAME=$(basename "$path")
    SPEC_DIR=$(dirname "$path")
    CSUM_FILE="$SPEC_NAME.sha256"
    CSUM_CHANGED=0
    RPMBUILD_OPTS_FILE="$SPEC_NAME.rpmbuild_opts"

    if [[ "$MODULARITY" != "true" && "$SPEC_NAME" =~ '.module' ]]; then
        echo "Not building modular $path."
        continue
    fi

    # detect spec change -> force rebuild
    pushd "$SPEC_DIR" > /dev/null
    if [ -f "$CSUM_FILE" ]; then
        sha256sum -c --status "$CSUM_FILE" || CSUM_CHANGED=1
    else
        CSUM_CHANGED=1
    fi
    popd > /dev/null

    RPMBUILD_CMD="rpmbuild --quiet -ba --nodeps"
    RPMBUILD_CMD="$RPMBUILD_CMD --define='_srcrpmdir $REPODIR/$REPO/src' --define='_rpmdir $REPODIR/$REPO'"
    RPMBUILD_CMD="$RPMBUILD_CMD --define='dist $DIST'"
    RPMBUILD_CMD="$RPMBUILD_CMD --define '_source_payload w1.gzdio' --define '_binary_payload w1.gzdio'"

    # add build options configured per spec file
    pushd "$SPEC_DIR" > /dev/null
        if [ -f "$RPMBUILD_OPTS_FILE" ]; then
            while IFS='' read -r CFGLINE || [ -n "${CFGLINE}" ]; do
                RPMBUILD_CMD="$RPMBUILD_CMD ${CFGLINE}"
            done < "$RPMBUILD_OPTS_FILE"
        fi
    popd > /dev/null

    # rebuild changed or new specs
    if [ $CSUM_CHANGED -eq 1 ]; then
        echo "Building $path..."
        eval "$RPMBUILD_CMD" --target=$ARCH "'$path'"

        # In addition to default $ARCH (x86_64) also build packages with architectures
        # whose names are contained in the specfile name
        if [[ "$SPEC_NAME" =~ "i686" ]]; then
            eval "$RPMBUILD_CMD" --target=i686 "'$path'"
        fi

        if [[ "$SPEC_NAME" =~ "i386" ]]; then
            eval "$RPMBUILD_CMD" --target=i386 "'$path'"
        fi

        if [[ "$SPEC_NAME" =~ "ppc64" ]]; then
            eval "$RPMBUILD_CMD" --target=ppc64 "'$path'"
        fi

        pushd "$SPEC_DIR" > /dev/null
        echo "Spec has changed, writing new checksum: $path"
        sha256sum "$(basename "$path")" > "$CSUM_FILE"
        popd > /dev/null
    fi

done


python3 "${GPGDIR}/sign.py"
"${DIR}/break-packages.sh"
"${CERTSDIR}/generate_certificates.sh"


# create a repo with broken rpm signatures
mkdir -p "${REPODIR}"/dnf-ci-broken-rpm-signature
"${DIR}"/break-rpm-signatures.py "${REPODIR}"/dnf-ci-gpg/noarch/setup-2.12.1-1.fc29.noarch.rpm "${REPODIR}"/dnf-ci-broken-rpm-signature/setup-2.12.1-1.fc29.noarch.rpm


echo "DONE: Test data created"
