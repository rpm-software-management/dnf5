BUILD_DIR ?= build
SHELL := bash
CMAKE_ARGS ?=
CTEST_ARGS ?=
NPROC ?= $(shell nproc)
RPMS_DIR = integration-tests/build/rpms
SOURCE_TARBALL_DIR = integration-tests/build/source
SRPM_DIR = integration-tests/build/srpm
STAMPS_DIR = integration-tests/build/stamps
RPM_BUILDDIR = integration-tests/build/builddir
VERSION := $(shell rpmspec -q --queryformat '%{VERSION}\n' dnf5.spec | head -n1)
SOURCE_TARBALL_PREFIX := dnf5-$(VERSION)
SOURCE_TARBALL_NAME := $(SOURCE_TARBALL_PREFIX).tar.gz
SOURCE_TARBALL_PATH := $(SOURCE_TARBALL_DIR)/$(SOURCE_TARBALL_NAME)
MOCK_CONFIG ?= fedora-rawhide-x86_64
CI_BASE_IMAGE ?= registry.fedoraproject.org/fedora:rawhide
CONTAINER_TEST = ./integration-tests/container-test

.ONESHELL:

FORCE:

.PHONY: help
help:
	@echo "Unit tests:"
	echo "  make build     - Configure and build the project"
	echo "  make test-unit - Run C++ unit tests (builds first)"
	echo "  make test      - Alias for test-unit"
	echo ""
	echo "Integration tests:"
	echo "  make test-integration       - Build RPMs (if needed), container (if needed), and run tests"
	echo "  make test-integration-build - Build RPMs and the test container (with change detection)"
	echo "  make rpms                   - Build RPMs from local source"
	echo "  make rpms-mock              - Build RPMs using Mock"
	echo "  make srpm                   - Build SRPM from local source"
	echo ""
	echo "  The test-integration target passes extra args to container-test via ARGS=, e.g.:"
	echo "    make test-integration ARGS='--command dnf5 --tags dnf5 config.feature'"
	echo "    make test-integration ARGS='--no-destructive --command dnf5'"
	echo "    make test-integration ARGS='-r --command dnf5 install.feature'"
	echo ""
	echo "Other:"
	echo "  make clean - Remove build directory, local RPMs, and stamps"
	echo ""
	echo "Variables:"
	echo "  BUILD_DIR=build                                         - Build directory (default: build)"
	echo "  CMAKE_ARGS=                                             - Extra cmake configure arguments"
	echo "  CTEST_ARGS=                                             - Extra ctest arguments (e.g. -R libdnf5)"
	echo "  NPROC=$$(nproc)                                         - Parallel build jobs"
	echo "  MOCK_CONFIG=fedora-rawhide-x86_64                       - Mock config for rpms-mock"
	echo "  CI_BASE_IMAGE=registry.fedoraproject.org/fedora:rawhide - Base image for test container"

.PHONY: build
build:
	cmake -B "$(BUILD_DIR)" -DWITH_TESTS=ON $(CMAKE_ARGS)
	cmake --build "$(BUILD_DIR)" -j"$(NPROC)"

.PHONY: test-unit
test-unit: build
	cd "$(BUILD_DIR)" && ctest --output-on-failure $(CTEST_ARGS)

.PHONY: test
test: test-unit

$(STAMPS_DIR)/MOCK_CONFIG: FORCE
	@mkdir -p "$(STAMPS_DIR)"
	echo -n "$(MOCK_CONFIG)" | cmp -s "$@" || echo -n "$(MOCK_CONFIG)" > "$@"

$(STAMPS_DIR)/CI_BASE_IMAGE: FORCE
	@mkdir -p "$(STAMPS_DIR)"
	echo -n "$(CI_BASE_IMAGE)" | cmp -s "$@" || echo -n "$(CI_BASE_IMAGE)" > "$@"

$(SOURCE_TARBALL_PATH): FORCE
	@set -e
	mkdir -p "$(SOURCE_TARBALL_DIR)"
	# Disregard changes in integration-tests.
	if git status --untracked-files=no --porcelain | cut -c 4- | grep -v '^integration-tests/' | grep -q .; then
		echo "Error: Working directory is not clean. Commit or stash your changes first."
		echo "       git archive does not include uncommitted changes."
		git status --short --untracked-files=no
		exit 1
	fi
	git archive --mtime="1970-01-01T00:00:00" --format=tar --prefix="$(SOURCE_TARBALL_PREFIX)/" HEAD | gzip -n > "$@.tmp"
	cmp -s "$@.tmp" "$@" || mv "$@.tmp" "$@"
	rm -f "$@.tmp"

$(STAMPS_DIR)/rpms: $(SOURCE_TARBALL_PATH)
	set -e
	mkdir -p "$(RPMS_DIR)" "$(RPM_BUILDDIR)" "$(SRPM_DIR)" "$(STAMPS_DIR)"
	rm -rf "$(RPMS_DIR)"/*
	rm -rf "$(SRPM_DIR)"/*
	rm -rf "$(RPM_BUILDDIR)"/*
	rpmbuild -bb dnf5.spec \
		--define "_sourcedir $(CURDIR)/$(SOURCE_TARBALL_DIR)" \
		--define "_rpmdir $(CURDIR)/$(RPMS_DIR)" \
		--define "_srcrpmdir $(CURDIR)/$(SRPM_DIR)" \
		--define "_builddir $(CURDIR)/$(RPM_BUILDDIR)" \
		-D "version $(VERSION)"
	touch "$(STAMPS_DIR)/rpms"

.PHONY: rpms
rpms: $(STAMPS_DIR)/rpms

$(STAMPS_DIR)/srpm: $(SOURCE_TARBALL_PATH)
	set -e
	mkdir -p "$(SRPM_DIR)" "$(STAMPS_DIR)"
	rm -rf "$(SRPM_DIR)"/*
	rpmbuild -bs dnf5.spec \
		--define "_sourcedir $(CURDIR)/$(SOURCE_TARBALL_DIR)" \
		--define "_srcrpmdir $(CURDIR)/$(SRPM_DIR)" \
		-D "version $(VERSION)"
	touch "$(STAMPS_DIR)/srpm"

.PHONY: srpm
srpm: $(STAMPS_DIR)/srpm

$(STAMPS_DIR)/rpms-mock: $(SOURCE_TARBALL_PATH) $(STAMPS_DIR)/MOCK_CONFIG
	set -e
	mkdir -p "$(RPMS_DIR)" "$(SRPM_DIR)" "$(STAMPS_DIR)"
	rm -rf "$(RPMS_DIR)"/*
	rm -rf "$(SRPM_DIR)"/*
	rpmbuild -bs dnf5.spec \
		--define "_sourcedir $(CURDIR)/$(SOURCE_TARBALL_DIR)" \
		--define "_srcrpmdir $(CURDIR)/$(SRPM_DIR)" \
		-D "version $(VERSION)"
	mock -r "$(MOCK_CONFIG)" --rebuild "$(SRPM_DIR)/dnf5-$(VERSION)"*.src.rpm \
		--resultdir="$(CURDIR)/$(RPMS_DIR)" --no-cleanup-after
	rm -rf integration-tests/rpms/*
	while IFS= read -r rpm; do
		ln -f "$$rpm" integration-tests/rpms/
	done < <(find "$(RPMS_DIR)" -maxdepth 1 -name '*.rpm' ! -name '*.src.rpm')
	touch "$(STAMPS_DIR)/rpms-mock"

.PHONY: rpms-mock
rpms-mock: $(STAMPS_DIR)/rpms-mock

$(STAMPS_DIR)/integration-tests: FORCE
	set -e
	mkdir -p "$(STAMPS_DIR)"
	stamp="$$(git ls-files -z integration-tests ':!integration-tests/dnf-behave-tests' | xargs -0 sha256sum | sha256sum)"
	echo -n "$$stamp" | cmp -s "$@" || echo -n "$$stamp" > "$@"

$(STAMPS_DIR)/test-integration-build: $(STAMPS_DIR)/rpms-mock $(STAMPS_DIR)/CI_BASE_IMAGE $(STAMPS_DIR)/integration-tests
	mkdir -p "$(STAMPS_DIR)"
	"$(CONTAINER_TEST)" build --base="$(CI_BASE_IMAGE)"
	touch "$(STAMPS_DIR)/test-integration-build"

.PHONY: test-integration-build
test-integration-build: $(STAMPS_DIR)/test-integration-build

.PHONY: test-integration
test-integration: $(STAMPS_DIR)/test-integration-build
	$(CONTAINER_TEST) -d run $(ARGS)

.PHONY: clean
clean:
	rm -rf "$(BUILD_DIR)"
	rm -rf "$(RPMS_DIR)"
	rm -rf "$(SRPM_DIR)"
	rm -rf "$(RPM_BUILDDIR)"
	rm -rf "$(STAMPS_DIR)"
	rm -rf "$(SOURCE_TARBALL_DIR)"
	rm -rf integration-tests/rpms/*
