#!/bin/bash
set -e

PROG_PATH="$(dirname "$(readlink -f -- "$0")")"
# shellcheck source=dnf-behave-tests/fixtures/certificates/x509certgen
. "${PROG_PATH}/x509certgen"
rm -rf "$PROG_PATH/testcerts"
mkdir -p "$PROG_PATH/testcerts"
pushd "$PROG_PATH/testcerts"
x509KeyGen ca
x509KeyGen server
x509KeyGen client
x509KeyGen ca2
x509KeyGen server2
x509KeyGen client2
x509SelfSign -t ca ca
x509SelfSign -t ca ca2
x509CertSign -t webserver --CA ca server
x509CertSign -t webclient --CA ca client
x509CertSign -t webserver --CA ca2 server2
x509CertSign -t webclient --CA ca2 client2
popd
