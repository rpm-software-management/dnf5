#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/comps_impl.hpp>

extern "C" {
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/repo_comps.h>
}

#include <filesystem>
#include <iostream>

namespace libdnf::comps {


Comps::Comps(libdnf::Base & base) : base{base}, p_impl{new Impl()} {}


Comps::~Comps() {}


void Comps::load_installed() {
    // TODO(pkratoch): load real installed groups
    std::filesystem::path data_path = PROJECT_SOURCE_DIR "/test/libdnf/comps/data/";
    std::string mock_path = data_path / "core.xml";

    Pool * pool = p_impl->get_pool();
    Repo * repo = 0;
    Id repoid;
    // Search for repo named @System
    FOR_REPOS(repoid, repo) {
        if (!strcasecmp(repo->name, "@System")) {
            break;
        }
    }
    // If repo named @System doesn't exist, create new repo
    if (!repo) {
        repo = repo_create(pool, "@System");
    }

    FILE * xml_doc = fopen(mock_path.c_str(), "r");
    // Load comps from the file
    // TODO(pkratoch): libsolv doesn't support environments yet
    repo_add_comps(repo, xml_doc, 0);
    fclose(xml_doc);
}


void Comps::load_from_file(const std::string & path, const char * reponame) {
    Pool * pool = p_impl->get_pool();
    Repo * repo = 0;
    Repo * tmp_repo = 0;
    Id repoid;
    // Search for repo named reponame
    FOR_REPOS(repoid, tmp_repo) {
        if (!strcasecmp(tmp_repo->name, reponame)) {
            repo = tmp_repo;
            break;
        }
    }
    // If repo named reponame doesn't exist, create new repo
    if (!repo) {
        repo = repo_create(pool, reponame);
    }

    FILE * xml_doc = fopen(path.c_str(), "r");
    // Load comps from the file
    // TODO(pkratoch): libsolv doesn't support environments yet
    repo_add_comps(repo, xml_doc, 0);
    fclose(xml_doc);
}


}  // namespace libdnf::comps

