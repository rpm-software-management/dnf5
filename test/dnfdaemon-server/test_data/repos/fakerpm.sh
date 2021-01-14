#!/bin/bash
#
# From: https://www.redhat.com/archives/rpm-list/2006-November/msg00062.html
#
# Generate dummy/fake RPMs - great for acceptance testing systems
#
NAME=$1 
DEST_DIR=$2

#
# GEnerate Provides:
PROVIDES="Provides: ${1}"

#
# Generate Spec file
SPECFILE=$(mktemp)
cat <<EOF > ${SPECFILE}
#----------- spec file starts ---------------
Name:                   ${NAME}
Version:                1.0.1
Release:                0
Vendor:                 dummy
Group:                  dummy
Summary:                Provides %{name}
License:                %{vendor}
# in Provides: you add whatever you want to fool the system
Buildroot:              %{_tmppath}/%{name}-%{version}-root
${PROVIDES}
Provides: the	double	space.txt

%description
%{summary}

%files
EOF

#
# Build it 
BUILD_LOG=$(mktemp)
rpmbuild --define '_rpmdir /tmp' -bb "${SPECFILE}" > "${BUILD_LOG}"
if [ $? != 0 ]
then
  echo "ERROR: Could nto build dummy rpm!"
fi
PKG=$(awk '/^Wrote:/ { print $2 }' < "${BUILD_LOG}" )
rm "${BUILD_LOG}"

#
# Install it:
#rpm -Uvh "${PKG}"

rm "${SPECFILE}"

echo "DONE!  created ${PKG} and moved it here..."
mv $PKG ${DEST_DIR}
