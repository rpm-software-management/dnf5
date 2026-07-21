BUILD_DIR ?= build
CMAKE_ARGS ?=
CTEST_ARGS ?=
NPROC ?= $(shell nproc)
RPMS_DIR = integration-tests/rpms
VERSION = $(shell rpmspec -q --queryformat '%{VERSION}\n' dnf5.spec | head -n1)
CONTAINER_TEST = ./integration-tests/container-test

.PHONY: build test test-unit test-integration test-integration-build \
        test-integration-run rpms clean help

help:
	@echo "Unit tests:"
	@echo "  make build              - Configure and build the project"
	@echo "  make test-unit          - Run C++ unit tests (builds first)"
	@echo "  make test               - Alias for test-unit"
	@echo ""
	@echo "Integration tests:"
	@echo "  make test-integration        - Build local RPMs, container, and run dnf5 tests"
	@echo "  make test-integration-build  - Build local RPMs and the test container"
	@echo "  make test-integration-run    - Run tests (container must be built)"
	@echo "  make rpms                    - Build RPMs from local source"
	@echo ""
	@echo "  All integration targets pass extra args to container-test via ARGS=, e.g.:"
	@echo "    make test-integration ARGS='--command dnf5 --tags dnf5 config.feature'"
	@echo "    make test-integration-run ARGS='--no-destructive --command dnf5'"
	@echo "    make test-integration-run ARGS='-r --command dnf5 install.feature'"
	@echo ""
	@echo "Other:"
	@echo "  make clean              - Remove build directory and local RPMs"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_DIR=build         - Build directory (default: build)"
	@echo "  CMAKE_ARGS=             - Extra cmake configure arguments"
	@echo "  CTEST_ARGS=             - Extra ctest arguments (e.g. -R libdnf5)"
	@echo "  NPROC=$$(nproc)           - Parallel build jobs"

build:
	cmake -B $(BUILD_DIR) -DWITH_TESTS=ON $(CMAKE_ARGS)
	cmake --build $(BUILD_DIR) -j$(NPROC)

test-unit: build
	cd $(BUILD_DIR) && ctest --output-on-failure $(CTEST_ARGS)

test: test-unit

rpms:
	rm -f $(RPMS_DIR)/*.rpm
	git archive --format=tar.gz --prefix=dnf5-$(VERSION)/ HEAD -o /tmp/dnf5-$(VERSION).tar.gz
	rpmbuild -bb dnf5.spec \
		--define "_sourcedir /tmp" \
		--define "_rpmdir $(CURDIR)/$(RPMS_DIR)" \
		--define "_srcrpmdir /tmp" \
		--define "_builddir /tmp/dnf5-rpmbuild" \
		-D "version $(VERSION)"
	find $(RPMS_DIR) -mindepth 2 -name '*.rpm' -exec mv {} $(RPMS_DIR)/ \;
	rm -rf $(RPMS_DIR)/*/

test-integration-build: rpms
	$(CONTAINER_TEST) build

test-integration-run:
	$(CONTAINER_TEST) run $(ARGS)

test-integration: test-integration-build
	$(CONTAINER_TEST) run $(ARGS)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(RPMS_DIR)/*.rpm
