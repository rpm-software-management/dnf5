//
// Created by charlie on 8/18/23.
//

#include "test_repo_sack.hpp"

void RepoSackTest::test_call_twice_fails() {
    // Call this once...
    repo_sack->update_and_load_enabled_repos(true);

    // calling this again should fail
    CPPUNIT_ASSERT_THROW(repo_sack->update_and_load_enabled_repos(true), libdnf5::UserAssertionError);
}
