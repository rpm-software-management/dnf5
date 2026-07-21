%undefine _debuginfo_subpackages

Name:           nodejs
Epoch:          1
Version:        11.1.0
Release:        1.module_2379+8d497405

License:        MIT and ASL 2.0 and ISC and BSD
URL:            http://nodejs.org/

Summary:        JavaScript runtime

Provides:       bundled(c-ares) = 1.14.0
Provides:       nodejs-punycode = 2.1.0
Provides:       npm(punycode) = 2.1.0
Provides:       nodejs = 1:11.1.0-1.module_2379+8d497405
Provides:       nodejs(x86-64) = 1:11.1.0-1.module_2379+8d497405
Provides:       bundled(icu) = 63.1
Provides:       bundled(v8) = 7.0.276.32
Provides:       nodejs(abi) = 11.1
Provides:       nodejs(abi11) = 11.1
Provides:       nodejs(engine) = 11.1.0
Provides:       nodejs(v8-abi) = 7.0
Provides:       nodejs(v8-abi7) = 7.0

Requires:       rtld(GNU_HASH)
Requires:       wget
Requires:       postgresql

Conflicts:      node <= 0.3.2-12

Recommends:     npm = 1:6.4.1-1.11.1.0.1.module_2379+8d497405

%description
Node.js is a platform built on Chrome's JavaScript runtime
for easily building fast, scalable network applications.
Node.js uses an event-driven, non-blocking I/O model that
makes it lightweight and efficient, perfect for data-intensive
real-time applications that run across distributed devices.

%package devel
Summary:        JavaScript runtime - development headers

Provides:       nodejs-devel = 1:11.1.0-1.module_2379+8d497405
Provides:       nodejs-devel(x86-64) = 1:11.1.0-1.module_2379+8d497405

Requires:       rtld(GNU_HASH)
Requires:       nodejs(x86-64) = 1:11.1.0-1.module_2379+8d497405

%description devel
Development headers for the Node.js JavaScript runtime.

%package docs
Summary:        Node.js API documentation
BuildArch:      noarch

Provides:       nodejs-docs = 1:11.1.0-1.module_2379+8d497405

Conflicts:      nodejs < 1:11.1.0-1.module_2379+8d497405
Conflicts:      nodejs > 1:11.1.0-1.module_2379+8d497405

%description docs
The API documentation for the Node.js JavaScript runtime.

%package -n npm
Summary:        Node.js Package Manager

Provides:       bundled(nodejs-ajv) = 5.5.2
Provides:       bundled(nodejs-ansi-regex) = 3.0.0
Provides:       bundled(nodejs-ansi-styles) = 3.2.1
Provides:       bundled(nodejs-asap) = 2.0.6
Provides:       bundled(nodejs-assert-plus) = 1.0.0
Provides:       bundled(nodejs-asynckit) = 0.4.0
Provides:       bundled(nodejs-aws-sign2) = 0.7.0
Provides:       bundled(nodejs-balanced-match) = 1.0.0
Provides:       bundled(nodejs-bcrypt-pbkdf) = 1.0.2
Provides:       bundled(nodejs-brace-expansion) = 1.1.11
Provides:       bundled(nodejs-camelcase) = 4.1.0
Provides:       bundled(nodejs-caseless) = 0.12.0
Provides:       bundled(nodejs-chalk) = 2.4.1
Provides:       bundled(nodejs-chownr) = 1.0.1
Provides:       bundled(nodejs-co) = 4.6.0
Provides:       bundled(nodejs-combined-stream) = 1.0.6
Provides:       bundled(nodejs-concat-map) = 0.0.1
Provides:       bundled(nodejs-core-util-is) = 1.0.2
Provides:       bundled(nodejs-dashdash) = 1.14.1
Provides:       bundled(nodejs-debug) = 3.1.0
Provides:       bundled(nodejs-decode-uri-component) = 0.2.0
Provides:       bundled(nodejs-delayed-stream) = 1.0.0
Provides:       bundled(nodejs-detect-indent) = 5.0.0
Provides:       bundled(nodejs-duplexify) = 3.6.0
Provides:       bundled(nodejs-ecc-jsbn) = 0.1.2
Provides:       bundled(nodejs-end-of-stream) = 1.4.1
Provides:       bundled(nodejs-escape-string-regexp) = 1.0.5
Provides:       bundled(nodejs-extend) = 3.0.2
Provides:       bundled(nodejs-extsprintf) = 1.3.0
Provides:       bundled(nodejs-fast-deep-equal) = 1.1.0
Provides:       bundled(nodejs-fast-json-stable-stringify) = 2.0.0
Provides:       bundled(nodejs-forever-agent) = 0.6.1
Provides:       bundled(nodejs-form-data) = 2.3.2
Provides:       bundled(nodejs-fs.realpath) = 1.0.0
Provides:       bundled(nodejs-getpass) = 0.1.7
Provides:       bundled(nodejs-glob) = 7.1.2
Provides:       bundled(nodejs-graceful-fs) = 4.1.11
Provides:       bundled(nodejs-har-schema) = 2.0.0
Provides:       bundled(nodejs-has-flag) = 3.0.0
Provides:       bundled(nodejs-http-signature) = 1.2.0
Provides:       bundled(nodejs-iconv-lite) = 0.4.23
Provides:       bundled(nodejs-inflight) = 1.0.6
Provides:       bundled(nodejs-inherits) = 2.0.3
Provides:       bundled(nodejs-ini) = 1.3.5
Provides:       bundled(nodejs-is-ci) = 1.1.0
Provides:       bundled(nodejs-is-fullwidth-code-point) = 2.0.0
Provides:       bundled(nodejs-is-typedarray) = 1.0.0
Provides:       bundled(nodejs-isarray) = 1.0.0
Provides:       bundled(nodejs-isstream) = 0.1.2
Provides:       bundled(nodejs-jsbn) = 0.1.1
Provides:       bundled(nodejs-json-schema) = 0.2.3
Provides:       bundled(nodejs-json-schema-traverse) = 0.3.1
Provides:       bundled(nodejs-json-stringify-safe) = 5.0.1
Provides:       bundled(nodejs-jsprim) = 1.4.1
Provides:       bundled(nodejs-mime-db) = 1.35.0
Provides:       bundled(nodejs-mime-types) = 2.1.19
Provides:       bundled(nodejs-mimic-fn) = 1.2.0
Provides:       bundled(nodejs-minimatch) = 3.0.4
Provides:       bundled(nodejs-minimist) = 0.0.8
Provides:       bundled(nodejs-mkdirp) = 0.5.1
Provides:       bundled(nodejs-ms) = 2.0.0
Provides:       bundled(nodejs-mute-stream) = 0.0.7
Provides:       bundled(nodejs-npm-logical-tree) = 1.2.1
Provides:       bundled(nodejs-object-assign) = 4.1.1
Provides:       bundled(nodejs-once) = 1.4.0
Provides:       bundled(nodejs-os-tmpdir) = 1.0.2
Provides:       bundled(nodejs-path-is-absolute) = 1.0.1
Provides:       bundled(nodejs-performance-now) = 2.1.0
Provides:       bundled(nodejs-process-nextick-args) = 2.0.0
Provides:       bundled(nodejs-pump) = 2.0.1
Provides:       bundled(nodejs-pumpify) = 1.5.1
Provides:       bundled(nodejs-punycode) = 1.4.1
Provides:       bundled(nodejs-qs) = 6.5.2
Provides:       bundled(nodejs-read) = 1.0.7
Provides:       bundled(nodejs-readable-stream) = 2.3.6
Provides:       bundled(nodejs-retry) = 0.10.1
Provides:       bundled(nodejs-rimraf) = 2.6.2
Provides:       bundled(nodejs-safe-buffer) = 5.1.2
Provides:       bundled(nodejs-safer-buffer) = 2.1.2
Provides:       bundled(nodejs-semver) = 5.5.0
Provides:       bundled(nodejs-signal-exit) = 3.0.2
Provides:       bundled(nodejs-spdx-correct) = 3.0.0
Provides:       bundled(nodejs-spdx-exceptions) = 2.1.0
Provides:       bundled(nodejs-spdx-expression-parse) = 3.0.0
Provides:       bundled(nodejs-spdx-license-ids) = 3.0.0
Provides:       bundled(nodejs-sshpk) = 1.14.2
Provides:       bundled(nodejs-stream-shift) = 1.0.0
Provides:       bundled(nodejs-string-width) = 2.1.1
Provides:       bundled(nodejs-string_decoder) = 1.1.1
Provides:       bundled(nodejs-strip-ansi) = 4.0.0
Provides:       bundled(nodejs-supports-color) = 5.4.0
Provides:       bundled(nodejs-through) = 2.3.8
Provides:       bundled(nodejs-through2) = 2.0.3
Provides:       bundled(nodejs-tunnel-agent) = 0.6.0
Provides:       bundled(nodejs-tweetnacl) = 0.14.5
Provides:       bundled(nodejs-util-deprecate) = 1.0.2
Provides:       bundled(nodejs-uuid) = 3.3.2
Provides:       bundled(nodejs-verror) = 1.10.0
Provides:       bundled(nodejs-wrappy) = 1.0.2
Provides:       bundled(nodejs-xtend) = 4.0.1
Provides:       npm = 1:6.4.1
Provides:       npm(npm) = 6.4.1
Provides:       bundled(nodejs-JSONStream) = 1.3.4
Provides:       bundled(nodejs-abbrev) = 1.1.1
Provides:       bundled(nodejs-agent-base) = 4.2.0
Provides:       bundled(nodejs-agentkeepalive) = 3.4.1
Provides:       bundled(nodejs-ansi-align) = 2.0.0
Provides:       bundled(nodejs-ansi-regex) = 2.1.1
Provides:       bundled(nodejs-ansicolors) = 0.3.2
Provides:       bundled(nodejs-ansistyles) = 0.1.3
Provides:       bundled(nodejs-aproba) = 1.2.0
Provides:       bundled(nodejs-archy) = 1.0.0
Provides:       bundled(nodejs-are-we-there-yet) = 1.1.4
Provides:       bundled(nodejs-asn1) = 0.2.4
Provides:       bundled(nodejs-aws4) = 1.8.0
Provides:       bundled(nodejs-bin-links) = 1.1.2
Provides:       bundled(nodejs-block-stream) = 0.0.9
Provides:       bundled(nodejs-bluebird) = 3.5.1
Provides:       bundled(nodejs-boxen) = 1.3.0
Provides:       bundled(nodejs-buffer-from) = 1.0.0
Provides:       bundled(nodejs-builtin-modules) = 1.1.1
Provides:       bundled(nodejs-builtins) = 1.0.3
Provides:       bundled(nodejs-byline) = 5.0.0
Provides:       bundled(nodejs-byte-size) = 4.0.3
Provides:       bundled(nodejs-cacache) = 10.0.4
Provides:       bundled(nodejs-cacache) = 11.2.0
Provides:       bundled(nodejs-call-limit) = 1.1.0
Provides:       bundled(nodejs-capture-stack-trace) = 1.0.0
Provides:       bundled(nodejs-ci-info) = 1.4.0
Provides:       bundled(nodejs-cidr-regex) = 2.0.9
Provides:       bundled(nodejs-cli-boxes) = 1.0.0
Provides:       bundled(nodejs-cli-columns) = 3.1.2
Provides:       bundled(nodejs-cli-table3) = 0.5.0
Provides:       bundled(nodejs-cliui) = 4.1.0
Provides:       bundled(nodejs-clone) = 1.0.4
Provides:       bundled(nodejs-cmd-shim) = 2.0.2
Provides:       bundled(nodejs-code-point-at) = 1.1.0
Provides:       bundled(nodejs-color-convert) = 1.9.1
Provides:       bundled(nodejs-color-name) = 1.1.3
Provides:       bundled(nodejs-colors) = 1.1.2
Provides:       bundled(nodejs-columnify) = 1.5.4
Provides:       bundled(nodejs-concat-stream) = 1.6.2
Provides:       bundled(nodejs-config-chain) = 1.1.11
Provides:       bundled(nodejs-configstore) = 3.1.2
Provides:       bundled(nodejs-console-control-strings) = 1.1.0
Provides:       bundled(nodejs-copy-concurrently) = 1.0.5
Provides:       bundled(nodejs-create-error-class) = 3.0.2
Provides:       bundled(nodejs-cross-spawn) = 5.1.0
Provides:       bundled(nodejs-crypto-random-string) = 1.0.0
Provides:       bundled(nodejs-cyclist) = 0.2.2
Provides:       bundled(nodejs-debuglog) = 1.0.1
Provides:       bundled(nodejs-decamelize) = 1.2.0
Provides:       bundled(nodejs-deep-extend) = 0.5.1
Provides:       bundled(nodejs-defaults) = 1.0.3
Provides:       bundled(nodejs-delegates) = 1.0.0
Provides:       bundled(nodejs-detect-newline) = 2.1.0
Provides:       bundled(nodejs-dezalgo) = 1.0.3
Provides:       bundled(nodejs-dot-prop) = 4.2.0
Provides:       bundled(nodejs-dotenv) = 5.0.1
Provides:       bundled(nodejs-duplexer3) = 0.1.4
Provides:       bundled(nodejs-editor) = 1.0.0
Provides:       bundled(nodejs-encoding) = 0.1.12
Provides:       bundled(nodejs-err-code) = 1.1.2
Provides:       bundled(nodejs-errno) = 0.1.7
Provides:       bundled(nodejs-es6-promise) = 4.2.4
Provides:       bundled(nodejs-es6-promisify) = 5.0.0
Provides:       bundled(nodejs-execa) = 0.7.0
Provides:       bundled(nodejs-figgy-pudding) = 2.0.1
Provides:       bundled(nodejs-figgy-pudding) = 3.4.1
Provides:       bundled(nodejs-find-npm-prefix) = 1.0.2
Provides:       bundled(nodejs-find-up) = 2.1.0
Provides:       bundled(nodejs-flush-write-stream) = 1.0.3
Provides:       bundled(nodejs-from2) = 1.3.0
Provides:       bundled(nodejs-from2) = 2.3.0
Provides:       bundled(nodejs-fs-minipass) = 1.2.5
Provides:       bundled(nodejs-fs-vacuum) = 1.2.10
Provides:       bundled(nodejs-fs-write-stream-atomic) = 1.0.10
Provides:       bundled(nodejs-fstream) = 1.0.11
Provides:       bundled(nodejs-gauge) = 2.7.4
Provides:       bundled(nodejs-genfun) = 4.0.1
Provides:       bundled(nodejs-gentle-fs) = 2.0.1
Provides:       bundled(nodejs-get-caller-file) = 1.0.2
Provides:       bundled(nodejs-get-stream) = 3.0.0
Provides:       bundled(nodejs-global-dirs) = 0.1.1
Provides:       bundled(nodejs-got) = 6.7.1
Provides:       bundled(nodejs-har-validator) = 5.1.0
Provides:       bundled(nodejs-has-unicode) = 2.0.1
Provides:       bundled(nodejs-hosted-git-info) = 2.7.1
Provides:       bundled(nodejs-http-cache-semantics) = 3.8.1
Provides:       bundled(nodejs-http-proxy-agent) = 2.1.0
Provides:       bundled(nodejs-https-proxy-agent) = 2.2.1
Provides:       bundled(nodejs-humanize-ms) = 1.2.1
Provides:       bundled(nodejs-iferr) = 0.1.5
Provides:       bundled(nodejs-iferr) = 1.0.2
Provides:       bundled(nodejs-ignore-walk) = 3.0.1
Provides:       bundled(nodejs-import-lazy) = 2.1.0
Provides:       bundled(nodejs-imurmurhash) = 0.1.4
Provides:       bundled(nodejs-init-package-json) = 1.10.3
Provides:       bundled(nodejs-invert-kv) = 1.0.0
Provides:       bundled(nodejs-ip) = 1.1.5
Provides:       bundled(nodejs-ip-regex) = 2.1.0
Provides:       bundled(nodejs-is-builtin-module) = 1.0.0
Provides:       bundled(nodejs-is-cidr) = 2.0.6
Provides:       bundled(nodejs-is-fullwidth-code-point) = 1.0.0
Provides:       bundled(nodejs-is-installed-globally) = 0.1.0
Provides:       bundled(nodejs-is-npm) = 1.0.0
Provides:       bundled(nodejs-is-obj) = 1.0.1
Provides:       bundled(nodejs-is-path-inside) = 1.0.1
Provides:       bundled(nodejs-is-redirect) = 1.0.0
Provides:       bundled(nodejs-is-retry-allowed) = 1.1.0
Provides:       bundled(nodejs-is-stream) = 1.1.0
Provides:       bundled(nodejs-isarray) = 0.0.1
Provides:       bundled(nodejs-isexe) = 2.0.0
Provides:       bundled(nodejs-json-parse-better-errors) = 1.0.2
Provides:       bundled(nodejs-jsonparse) = 1.3.1
Provides:       bundled(nodejs-latest-version) = 3.1.0
Provides:       bundled(nodejs-lazy-property) = 1.0.0
Provides:       bundled(nodejs-lcid) = 1.0.0
Provides:       bundled(nodejs-libcipm) = 2.0.2
Provides:       bundled(nodejs-libnpmhook) = 4.0.1
Provides:       bundled(nodejs-libnpx) = 10.2.0
Provides:       bundled(nodejs-locate-path) = 2.0.0
Provides:       bundled(nodejs-lock-verify) = 2.0.2
Provides:       bundled(nodejs-lockfile) = 1.0.4
Provides:       bundled(nodejs-lodash._baseindexof) = 3.1.0
Provides:       bundled(nodejs-lodash._baseuniq) = 4.6.0
Provides:       bundled(nodejs-lodash._bindcallback) = 3.0.1
Provides:       bundled(nodejs-lodash._cacheindexof) = 3.0.2
Provides:       bundled(nodejs-lodash._createcache) = 3.1.2
Provides:       bundled(nodejs-lodash._createset) = 4.0.3
Provides:       bundled(nodejs-lodash._getnative) = 3.9.1
Provides:       bundled(nodejs-lodash._root) = 3.0.1
Provides:       bundled(nodejs-lodash.clonedeep) = 4.5.0
Provides:       bundled(nodejs-lodash.restparam) = 3.6.1
Provides:       bundled(nodejs-lodash.union) = 4.6.0
Provides:       bundled(nodejs-lodash.uniq) = 4.5.0
Provides:       bundled(nodejs-lodash.without) = 4.4.0
Provides:       bundled(nodejs-lowercase-keys) = 1.0.1
Provides:       bundled(nodejs-lru-cache) = 4.1.3
Provides:       bundled(nodejs-make-dir) = 1.3.0
Provides:       bundled(nodejs-make-fetch-happen) = 3.0.0
Provides:       bundled(nodejs-make-fetch-happen) = 4.0.1
Provides:       bundled(nodejs-meant) = 1.0.1
Provides:       bundled(nodejs-mem) = 1.1.0
Provides:       bundled(nodejs-minimist) = 1.2.0
Provides:       bundled(nodejs-minipass) = 2.3.3
Provides:       bundled(nodejs-minizlib) = 1.1.0
Provides:       bundled(nodejs-mississippi) = 2.0.0
Provides:       bundled(nodejs-mississippi) = 3.0.0
Provides:       bundled(nodejs-move-concurrently) = 1.0.1
Provides:       bundled(nodejs-ms) = 2.1.1
Provides:       bundled(nodejs-node-fetch-npm) = 2.0.2
Provides:       bundled(nodejs-node-gyp) = 3.8.0
Provides:       bundled(nodejs-nopt) = 3.0.6
Provides:       bundled(nodejs-nopt) = 4.0.1
Provides:       bundled(nodejs-normalize-package-data) = 2.4.0
Provides:       bundled(nodejs-npm-audit-report) = 1.3.1
Provides:       bundled(nodejs-npm-bundled) = 1.0.5
Provides:       bundled(nodejs-npm-cache-filename) = 1.0.2
Provides:       bundled(nodejs-npm-install-checks) = 3.0.0
Provides:       bundled(nodejs-npm-lifecycle) = 2.1.0
Provides:       bundled(nodejs-npm-package-arg) = 6.1.0
Provides:       bundled(nodejs-npm-packlist) = 1.1.11
Provides:       bundled(nodejs-npm-pick-manifest) = 2.1.0
Provides:       bundled(nodejs-npm-profile) = 3.0.2
Provides:       bundled(nodejs-npm-registry-client) = 8.6.0
Provides:       bundled(nodejs-npm-registry-fetch) = 1.1.0
Provides:       bundled(nodejs-npm-registry-fetch) = 3.1.1
Provides:       bundled(nodejs-npm-run-path) = 2.0.2
Provides:       bundled(nodejs-npm-user-validate) = 1.0.0
Provides:       bundled(nodejs-npmlog) = 4.1.2
Provides:       bundled(nodejs-number-is-nan) = 1.0.1
Provides:       bundled(nodejs-oauth-sign) = 0.9.0
Provides:       bundled(nodejs-opener) = 1.5.0
Provides:       bundled(nodejs-os-homedir) = 1.0.2
Provides:       bundled(nodejs-os-locale) = 2.1.0
Provides:       bundled(nodejs-osenv) = 0.1.5
Provides:       bundled(nodejs-p-finally) = 1.0.0
Provides:       bundled(nodejs-p-limit) = 1.2.0
Provides:       bundled(nodejs-p-locate) = 2.0.0
Provides:       bundled(nodejs-p-try) = 1.0.0
Provides:       bundled(nodejs-package-json) = 4.0.1
Provides:       bundled(nodejs-pacote) = 8.1.6
Provides:       bundled(nodejs-parallel-transform) = 1.1.0
Provides:       bundled(nodejs-path-exists) = 3.0.0
Provides:       bundled(nodejs-path-is-inside) = 1.0.2
Provides:       bundled(nodejs-path-key) = 2.0.1
Provides:       bundled(nodejs-pify) = 3.0.0
Provides:       bundled(nodejs-prepend-http) = 1.0.4
Provides:       bundled(nodejs-promise-inflight) = 1.0.1
Provides:       bundled(nodejs-promise-retry) = 1.1.1
Provides:       bundled(nodejs-promzard) = 0.3.0
Provides:       bundled(nodejs-proto-list) = 1.2.4
Provides:       bundled(nodejs-protoduck) = 5.0.0
Provides:       bundled(nodejs-prr) = 1.0.1
Provides:       bundled(nodejs-pseudomap) = 1.0.2
Provides:       bundled(nodejs-psl) = 1.1.29
Provides:       bundled(nodejs-pump) = 3.0.0
Provides:       bundled(nodejs-qrcode-terminal) = 0.12.0
Provides:       bundled(nodejs-query-string) = 6.1.0
Provides:       bundled(nodejs-qw) = 1.0.1
Provides:       bundled(nodejs-rc) = 1.2.7
Provides:       bundled(nodejs-read-cmd-shim) = 1.0.1
Provides:       bundled(nodejs-read-installed) = 4.0.3
Provides:       bundled(nodejs-read-package-json) = 2.0.13
Provides:       bundled(nodejs-read-package-tree) = 5.2.1
Provides:       bundled(nodejs-readable-stream) = 1.1.14
Provides:       bundled(nodejs-readdir-scoped-modules) = 1.0.2
Provides:       bundled(nodejs-registry-auth-token) = 3.3.2
Provides:       bundled(nodejs-registry-url) = 3.1.0
Provides:       bundled(nodejs-request) = 2.88.0
Provides:       bundled(nodejs-require-directory) = 2.1.1
Provides:       bundled(nodejs-require-main-filename) = 1.0.1
Provides:       bundled(nodejs-resolve-from) = 4.0.0
Provides:       bundled(nodejs-retry) = 0.12.0
Provides:       bundled(nodejs-run-queue) = 1.0.3
Provides:       bundled(nodejs-semver) = 5.3.0
Provides:       bundled(nodejs-semver-diff) = 2.1.0
Provides:       bundled(nodejs-set-blocking) = 2.0.0
Provides:       bundled(nodejs-sha) = 2.0.1
Provides:       bundled(nodejs-shebang-command) = 1.2.0
Provides:       bundled(nodejs-shebang-regex) = 1.0.0
Provides:       bundled(nodejs-slash) = 1.0.0
Provides:       bundled(nodejs-slide) = 1.1.6
Provides:       bundled(nodejs-smart-buffer) = 1.1.15
Provides:       bundled(nodejs-smart-buffer) = 4.0.1
Provides:       bundled(nodejs-socks) = 1.1.10
Provides:       bundled(nodejs-socks) = 2.2.0
Provides:       bundled(nodejs-socks-proxy-agent) = 3.0.1
Provides:       bundled(nodejs-socks-proxy-agent) = 4.0.1
Provides:       bundled(nodejs-sorted-object) = 2.0.1
Provides:       bundled(nodejs-sorted-union-stream) = 2.1.3
Provides:       bundled(nodejs-ssri) = 5.3.0
Provides:       bundled(nodejs-ssri) = 6.0.0
Provides:       bundled(nodejs-stream-each) = 1.2.2
Provides:       bundled(nodejs-stream-iterate) = 1.2.0
Provides:       bundled(nodejs-strict-uri-encode) = 2.0.0
Provides:       bundled(nodejs-string-width) = 1.0.2
Provides:       bundled(nodejs-string_decoder) = 0.10.31
Provides:       bundled(nodejs-stringify-package) = 1.0.0
Provides:       bundled(nodejs-strip-ansi) = 3.0.1
Provides:       bundled(nodejs-strip-eof) = 1.0.0
Provides:       bundled(nodejs-strip-json-comments) = 2.0.1
Provides:       bundled(nodejs-tar) = 2.2.1
Provides:       bundled(nodejs-tar) = 4.4.6
Provides:       bundled(nodejs-term-size) = 1.2.0
Provides:       bundled(nodejs-text-table) = 0.2.0
Provides:       bundled(nodejs-timed-out) = 4.0.1
Provides:       bundled(nodejs-tiny-relative-date) = 1.3.0
Provides:       bundled(nodejs-tough-cookie) = 2.4.3
Provides:       bundled(nodejs-typedarray) = 0.0.6
Provides:       bundled(nodejs-uid-number) = 0.0.6
Provides:       bundled(nodejs-umask) = 1.1.0
Provides:       bundled(nodejs-unique-filename) = 1.1.0
Provides:       bundled(nodejs-unique-slug) = 2.0.0
Provides:       bundled(nodejs-unique-string) = 1.0.0
Provides:       bundled(nodejs-unpipe) = 1.0.0
Provides:       bundled(nodejs-unzip-response) = 2.0.1
Provides:       bundled(nodejs-update-notifier) = 2.5.0
Provides:       bundled(nodejs-url-parse-lax) = 1.0.0
Provides:       bundled(nodejs-util-extend) = 1.0.3
Provides:       bundled(nodejs-validate-npm-package-license) = 3.0.4
Provides:       bundled(nodejs-validate-npm-package-name) = 3.0.0
Provides:       bundled(nodejs-wcwidth) = 1.0.1
Provides:       bundled(nodejs-which) = 1.3.1
Provides:       bundled(nodejs-which-module) = 2.0.0
Provides:       bundled(nodejs-wide-align) = 1.1.2
Provides:       bundled(nodejs-widest-line) = 2.0.0
Provides:       bundled(nodejs-worker-farm) = 1.6.0
Provides:       bundled(nodejs-wrap-ansi) = 2.1.0
Provides:       bundled(nodejs-write-file-atomic) = 2.3.0
Provides:       bundled(nodejs-xdg-basedir) = 3.0.0
Provides:       bundled(nodejs-y18n) = 3.2.1
Provides:       bundled(nodejs-y18n) = 4.0.0
Provides:       bundled(nodejs-yallist) = 2.1.2
Provides:       bundled(nodejs-yallist) = 3.0.2
Provides:       bundled(nodejs-yargs) = 11.0.0
Provides:       bundled(nodejs-yargs-parser) = 9.0.2
Provides:       npm = 1:6.4.1-1.11.1.0.1.module_2379+8d497405
Provides:       npm(x86-64) = 1:6.4.1-1.11.1.0.1.module_2379+8d497405

Requires:       nodejs = 1:11.1.0-1.module_2379+8d497405

Obsoletes:      npm < 3.5.4-6

%description -n npm
npm is a package manager for node.js. You can use it to install and publish
your node programs. It manages dependencies and does other cool stuff.

%files

%files devel

%files docs

%files -n npm

%changelog
