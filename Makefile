BUILD_DIR ?= build
CMAKE_ARGS ?=
CTEST_ARGS ?=
NPROC ?= $(shell nproc)
RPMS_DIR = integration-tests/rpms
SRPM_DIR = integration-tests/.srpm
STAMPS_DIR = integration-tests/.stamps
VERSION = $(shell rpmspec -q --queryformat '%{VERSION}\n' dnf5.spec | head -n1)
MOCK_CONFIG ?= fedora-43-x86_64
CONTAINER_TEST = ./integration-tests/container-test

.PHONY: build test test-unit test-integration test-integration-build \
        test-integration-run rpms rpms-mock srpm clean help check-clean-tree

help:
	@echo "Unit tests:"
	@echo "  make build              - Configure and build the project"
	@echo "  make test-unit          - Run C++ unit tests (builds first)"
	@echo "  make test               - Alias for test-unit"
	@echo ""
	@echo "Integration tests:"
	@echo "  make test-integration        - Build RPMs (if needed), container (if needed), and run tests"
	@echo "  make test-integration-build  - Build RPMs and the test container (with change detection)"
	@echo "  make test-integration-run    - Run tests (container must be built)"
	@echo "  make rpms                    - Build RPMs from local source (rpmbuild, no change detection)"
	@echo "  make rpms-mock               - Build RPMs using Mock (no change detection)"
	@echo "  make srpm                    - Build SRPM from local source"
	@echo ""
	@echo "  All integration targets pass extra args to container-test via ARGS=, e.g.:"
	@echo "    make test-integration ARGS='--command dnf5 --tags dnf5 config.feature'"
	@echo "    make test-integration-run ARGS='--no-destructive --command dnf5'"
	@echo "    make test-integration-run ARGS='-r --command dnf5 install.feature'"
	@echo ""
	@echo "Other:"
	@echo "  make clean              - Remove build directory, local RPMs, and stamps"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_DIR=build         - Build directory (default: build)"
	@echo "  CMAKE_ARGS=             - Extra cmake configure arguments"
	@echo "  CTEST_ARGS=             - Extra ctest arguments (e.g. -R libdnf5)"
	@echo "  NPROC=$$(nproc)           - Parallel build jobs"
	@echo "  MOCK_CONFIG=fedora-rawhide-x86_64  - Mock config for rpms-mock"

build:
	cmake -B $(BUILD_DIR) -DWITH_TESTS=ON $(CMAKE_ARGS)
	cmake --build $(BUILD_DIR) -j$(NPROC)

test-unit: build
	cd $(BUILD_DIR) && ctest --output-on-failure $(CTEST_ARGS)

test: test-unit

check-clean-tree:
	@if [ -n "$$(git status --porcelain)" ]; then \
		echo "Error: Working directory is not clean. Commit or stash your changes first."; \
		echo "       git archive does not include uncommitted changes."; \
		git status --short; \
		exit 1; \
	fi

srpm: check-clean-tree
	@mkdir -p $(SRPM_DIR)
	git archive --format=tar.gz --prefix=dnf5-$(VERSION)/ HEAD -o $(SRPM_DIR)/dnf5-$(VERSION).tar.gz

rpms: srpm
	@mkdir -p $(RPMS_DIR)
	rm -f $(RPMS_DIR)/*.rpm
	rpmbuild -bb dnf5.spec \
		--define "_sourcedir $(CURDIR)/$(SRPM_DIR)" \
		--define "_rpmdir $(CURDIR)/$(RPMS_DIR)" \
		--define "_srcrpmdir $(CURDIR)/$(SRPM_DIR)" \
		--define "_builddir /tmp/dnf5-rpmbuild" \
		-D "version $(VERSION)"
	find $(RPMS_DIR) -mindepth 2 -name '*.rpm' -exec mv {} $(RPMS_DIR)/ \;
	rm -rf $(RPMS_DIR)/*/

rpms-mock: srpm
	@mkdir -p $(RPMS_DIR)
	rm -f $(RPMS_DIR)/*.rpm
	rpmbuild -bs dnf5.spec \
		--define "_sourcedir $(CURDIR)/$(SRPM_DIR)" \
		--define "_srcrpmdir $(CURDIR)/$(SRPM_DIR)" \
		-D "version $(VERSION)"
	mock -r $(MOCK_CONFIG) --rebuild $(SRPM_DIR)/dnf5-$(VERSION)*.src.rpm \
		--resultdir=$(CURDIR)/$(RPMS_DIR) --no-cleanup-after
	rm -f $(RPMS_DIR)/*.src.rpm $(RPMS_DIR)/*.log $(RPMS_DIR)/installed_pkgs \
		$(RPMS_DIR)/root.log $(RPMS_DIR)/build.log $(RPMS_DIR)/state.log \
		$(RPMS_DIR)/hw_info.log $(RPMS_DIR)/available_pkgs

test-integration-build:
	@mkdir -p $(STAMPS_DIR)
	@commit=$$(git rev-parse HEAD); \
	if [ ! -f $(STAMPS_DIR)/rpms-$$commit ]; then \
		echo "==> RPMs out of date (commit $$commit), rebuilding with mock..."; \
		$(MAKE) rpms-mock; \
		rm -f $(STAMPS_DIR)/rpms-*; \
		touch $(STAMPS_DIR)/rpms-$$commit; \
	else \
		echo "==> RPMs up to date for commit $$commit, skipping rebuild."; \
	fi
	@tree=$$(git rev-parse HEAD:integration-tests); \
	rpms_stamp=$$(ls $(STAMPS_DIR)/rpms-* 2>/dev/null | head -1 | xargs basename 2>/dev/null); \
	stamp="$$tree-$$rpms_stamp"; \
	if [ ! -f $(STAMPS_DIR)/container-$$stamp ]; then \
		echo "==> Container out of date, rebuilding..."; \
		$(CONTAINER_TEST) build; \
		rm -f $(STAMPS_DIR)/container-*; \
		touch $(STAMPS_DIR)/container-$$stamp; \
	else \
		echo "==> Container up to date, skipping rebuild."; \
	fi

test-integration-run:
	$(CONTAINER_TEST) -d run $(ARGS)

test-integration: test-integration-build
	$(CONTAINER_TEST) -d run $(ARGS)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(RPMS_DIR)/*.rpm
	rm -rf $(SRPM_DIR)
	rm -rf $(STAMPS_DIR)
