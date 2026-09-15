#include "../../include/commands/RemoteCommands.h"
#include "../../include/commands/MergeCommands.h"
#include "../../include/Repository.h"
#include "../../include/ObjectStore.h"
#include "../../include/CommitGraph.h"
#include "../../include/Utils.h"

#include <cstdio>

namespace {

std::string remoteFile(const std::string& name) {
    return Utils::join(Repository::remotesDir(), name);
}

/** The stored .gitlite path of REMOTENAME, exiting with
 *  "Remote directory not found." if it is missing or not a directory. */
std::string requireRemoteDir(const std::string& name) {
    std::string path;
    if (Utils::isFile(remoteFile(name))) {
        path = Utils::readContentsAsString(remoteFile(name));
    }
    if (path.empty() || !Utils::isDirectory(path)) {
        Utils::exitWithMessage("Remote directory not found.");
    }
    return path;
}

} // namespace

void Commands::addRemote(const std::string& name, const std::string& path) {
    if (Utils::isFile(remoteFile(name))) {
        Utils::exitWithMessage("A remote with that name already exists.");
    }
    // Forward slashes are already the path separator on Linux.
    Utils::writeContents(remoteFile(name), path);
}

void Commands::rmRemote(const std::string& name) {
    if (!Utils::isFile(remoteFile(name))) {
        Utils::exitWithMessage("A remote with that name does not exist.");
    }
    remove(remoteFile(name).c_str());
}

void Commands::push(const std::string& remoteName, const std::string& branchName) {
    std::string remoteDir = requireRemoteDir(remoteName);
    std::string localHead = Repository::headCommitId();

    std::string remoteBranchFile = Utils::join(remoteDir, "refs/heads", branchName);
    if (Utils::isFile(remoteBranchFile)) {
        std::string remoteHead = Utils::readContentsAsString(remoteBranchFile);
        if (!CommitGraph::isAncestor(remoteHead, localHead)) {
            // The remote has commits the local history does not contain.
            Utils::exitWithMessage("Please pull down remote changes before pushing.");
        }
    }
    ObjectStore::copyObjects(Repository::getGitliteDir(), remoteDir);
    Utils::writeContents(remoteBranchFile, localHead);
}

void Commands::fetch(const std::string& remoteName, const std::string& branchName) {
    std::string remoteDir = requireRemoteDir(remoteName);

    std::string remoteBranchFile = Utils::join(remoteDir, "refs/heads", branchName);
    if (!Utils::isFile(remoteBranchFile)) {
        Utils::exitWithMessage("That remote does not have that branch.");
    }
    std::string remoteHead = Utils::readContentsAsString(remoteBranchFile);
    ObjectStore::copyObjects(remoteDir, Repository::getGitliteDir());
    // Track it locally as "<remote>/<branch>".
    Repository::setBranchHead(remoteName + "/" + branchName, remoteHead);
}

void Commands::pull(const std::string& remoteName, const std::string& branchName) {
    fetch(remoteName, branchName);
    merge(remoteName + "/" + branchName);
}
