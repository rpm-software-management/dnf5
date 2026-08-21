# PR Review Guidelines — dnf5

Review criteria for the dnf5 project. Intended for both human reviewers and automated review agents.


## Critical — likely to block merge

- **Correctness:** Logic errors, off-by-one mistakes, unhandled edge cases, incorrect use of standard library algorithms (e.g. preconditions not met).
- **Security:** Hardcoded secrets, improper input validation, unsafe parsing of external data (e.g. output piped from subprocesses), path traversal.
- **API/ABI compatibility:** Changes to public API or ABI — added/removed/reordered enum members, changed function signatures, modified class layouts, changed access modifiers — require a version bump (both `VERSION.cmake` and `dnf5.spec`). Prefer deprecation-then-removal over immediate breakage. Changing pass-by-value to pass-by-const-ref in a public header is also ABI-breaking. New public functions must have the appropriate visibility macro.
- **Error handling:** Swallowed exceptions, empty catch blocks, missing cleanup on error paths. Resources (file handles, connections, child processes) must be released even on failure — prefer RAII or scope-exit patterns. When calling C libraries, check return values for allocation failures. Exceptions should use project-specific types, not generic `std::runtime_error`.
- **Backward compatibility:** Changes must not silently alter default behavior for existing users or downstream consumers (daemon, other apps using libdnf5 as a library). Behavioral changes relative to DNF4 should be documented in `doc/changes_from_dnf4.7.rst`.
- **Referenced issues:** If the PR description or commits reference issues, read them and verify the changes correctly and completely address each one.
  - GitHub issues (e.g. `Fixes #123`, `Closes #456`, `Resolves owner/repo#789`): fetch using `gh issue view`.
  - Bugzilla bugs (full URLs like `https://bugzilla.redhat.com/show_bug.cgi?id=12345`, or shorthand like `RHBZ#12345`): fetch the full URL `https://bugzilla.redhat.com/show_bug.cgi?id=<number>`. Extract the bug summary, description, status, and key comments.
- **Spec file completeness:** When new files, subpackages, man pages, or directories are added, verify they are listed in the `%files` section of `dnf5.spec`.


## Important — should be addressed before merge

- **Documentation:** The most common review issue in this project. New commands, options, config keys, and behavior changes need corresponding updates to man pages (RST files in `doc/`), inline API doc comments, and changelog entries. Missing docs is a frequent cause of review iterations.
- **Internationalization (i18n):** All user-facing strings (error messages, prompts, output text) must be marked for translation. Watch for translatable strings that are glued together (breaking translator context) and missing translator comments for ambiguous strings. Library code should not print directly to stderr — use the logger or exceptions instead.
- **Test coverage:** New behavior should have tests — both unit tests and CI integration tests (ci-dnf-stack). Changed behavior should update existing tests. Reviewers consistently ask for integration test coverage alongside code changes.
- **Naming and wording:** Variable names, option names, command names, and user-facing text should be clear, grammatically correct, and consistent with existing conventions in the codebase. Follow established naming patterns for CLI options.
- **License and copyright headers:** New files must use `LGPL-2.1-or-later` with the short SPDX header format. Copyright headers should reference the current project name. Older files may have different licenses — only enforce LGPL-2.1-or-later for new files. See [CONTRIBUTING.md](CONTRIBUTING.md#license-header) for the exact format.
- **Code organization:** Logic that could be shared between CLI and library should live in the library. Avoid duplicating code that already exists elsewhere in the codebase. Non-trivial method implementations go in `.cpp` files, not headers (unless templates or trivial one-liners).
- **Commit messages:** Clear, descriptive, and explain "why" when the reason isn't obvious from the diff. Important context belongs in the commit message, not only in the PR description — commit messages go into release notes and are closer to the code than the PR management system. Unrelated changes should be in separate commits.
- **Version bumps:** When adding new public methods to libdnf5 or libdnf5-cli, bump the minor version in both `VERSION.cmake` and `dnf5.spec` — but only if the current minor version has already been tagged in a release.


## Suggestions — non-blocking improvements

- **Performance:** Unnecessary copies of large objects, missing move semantics, redundant operations. Prefer const references where possible.
- **Type safety:** Correct use of types (e.g. `size_t` vs raw integers, proper casting). Avoid unsafe casts when a type-correct API is available.
- **Const correctness:** Prefer `const` for variables, parameters, and methods where possible.
- **Code style:** Consistent formatting, proper use of namespace aliases, appropriate use of `static` for file-local variables.
- **Scope and lifetime:** Variables should be declared in the narrowest scope possible. Watch for values carried across loop iterations unnecessarily.
- **Deprecation handling:** Deprecated APIs should be marked with compiler attributes and documented with planned removal timelines.
- **Code sharing:** If logic is duplicated between components (dnf5 CLI, libdnf5, dnf5daemon-server), suggest moving shared code into the library. Consider filing a follow-up issue rather than blocking the PR.


## Review tone and communication

This team has a collaborative review culture:

- **Non-blocking by default.** Most feedback is delivered as suggestions, not demands. Reserve CHANGES_REQUESTED for correctness issues, API/ABI breaks, and missing critical documentation.
- **Distinguish severity clearly.** Prefix minor items with "Nit:" or similar markers. For issues that should not block the current PR, say so explicitly (e.g. "This could be improved in a follow-up PR").
- **Suggest follow-ups over blocking.** For out-of-scope improvements, suggest filing a follow-up issue rather than expanding the current PR.
- **Acknowledge unrelated CI failures.** When CI failures are not caused by the PR under review, note this explicitly to avoid unnecessary churn.
- **Guide external contributors patiently.** Explain project conventions (commit message style, formatting tools, license headers) rather than just requesting changes.
