#include "../include/Repository.h"
#include "../include/ObjectStore.h"
#include "../include/Utils.h"

#include <algorithm>
#include <dirent.h>

/* ---------------------------------------------------------------
 * Paths
 * --------------------------------------------------------------- */

std::string Repository::getGitliteDir() { return ".gitlite"; }

std::string Repository::headFile()   { return Utils::join(getGitliteDir(), "HEAD"); }
std::string Repository::headsDir()   { return Utils::join(getGitliteDir(), "refs/heads"); }
std::string Repository::objectsDir() { return Utils::join(getGitliteDir(), "objects"); }
std::string Repository::commitsDir() { return Utils::join(objectsDir(), "commits"); }
std::string Repository::blobsDir()   { return Utils::join(objectsDir(), "blobs"); }
std::string Repository::addedDir()   { return Utils::join(getGitliteDir(), "staging/added"); }
std::string Repository::removedDir() { return Utils::join(getGitliteDir(), "staging/removed"); }
std::string Repository::remotesDir() { return Utils::join(getGitliteDir(), "remotes"); }

std::string Repository::branchFile(const std::string& name) {
    return Utils::join(headsDir(), name);
}

void Repository::createLayout() {
    Utils::createDirectories(commitsDir());
    Utils::createDirectories(blobsDir());
    Utils::createDirectories(headsDir());
    Utils::createDirectories(addedDir());
    Utils::createDirectories(removedDir());
    Utils::createDirectories(remotesDir());
}

/* ---------------------------------------------------------------
 * HEAD and branch refs
 * --------------------------------------------------------------- */

std::string Repository::currentBranch() {
    return Utils::readContentsAsString(headFile());
}

void Repository::setCurrentBranch(const std::string& name) {
    Utils::writeContents(headFile(), name);
}

bool Repository::branchExists(const std::string& name) {
    return Utils::isFile(branchFile(name));
}

static void collectBranches(const std::string& dir, const std::string& prefix,
                            std::vector<std::string>& out) {
    DIR* d = opendir(dir.c_str());
    if (d == nullptr) {
        return;
    }
    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        std::string full = Utils::join(dir, name);
        if (Utils::isDirectory(full)) {
            // Remote-tracking branches such as "R1/master" nest one level.
            collectBranches(full, prefix + name + "/", out);
        } else {
            out.push_back(prefix + name);
        }
    }
    closedir(d);
}

std::vector<std::string> Repository::allBranches() {
    std::vector<std::string> result;
    collectBranches(headsDir(), "", result);
    std::sort(result.begin(), result.end());
    return result;
}

std::string Repository::branchHead(const std::string& name) {
    return Utils::readContentsAsString(branchFile(name));
}

void Repository::setBranchHead(const std::string& name, const std::string& commitId) {
    Utils::writeContents(branchFile(name), commitId);
}

void Repository::removeBranch(const std::string& name) {
    remove(branchFile(name).c_str());
}

std::string Repository::headCommitId() {
    return branchHead(currentBranch());
}

Commit Repository::headCommit() {
    return ObjectStore::readCommit(headCommitId());
}
