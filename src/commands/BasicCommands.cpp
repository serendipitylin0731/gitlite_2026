#include "../../include/commands/BasicCommands.h"
#include "../../include/Repository.h"
#include "../../include/ObjectStore.h"
#include "../../include/StagingArea.h"
#include "../../include/Commit.h"
#include "../../include/Utils.h"

#include <ctime>
#include <map>
#include <vector>

void Commands::init() {
    if (Utils::isDirectory(Repository::getGitliteDir())) {
        Utils::exitWithMessage(
            "A Gitlite version-control system already exists in the current directory.");
    }
    Repository::createLayout();

    // The initial commit is empty, sits at the Unix epoch and is shared
    // (same id) by every repository.
    Commit initial("initial commit", 0, {}, {});
    ObjectStore::writeCommit(initial);
    Repository::setBranchHead("master", initial.id());
    Repository::setCurrentBranch("master");
}

void Commands::add(const std::string& filename) {
    if (!Utils::isFile(filename)) {
        Utils::exitWithMessage("File does not exist.");
    }
    std::string contents = Utils::readContentsAsString(filename);
    std::string blobId = Utils::sha1(contents);

    Commit head = Repository::headCommit();
    if (head.tracks(filename) && head.blobId(filename) == blobId) {
        // Identical to the committed version: neither stage it nor keep
        // a pending removal mark for it.
        StagingArea::unstage(filename);
        return;
    }
    StagingArea::stageAddition(filename, contents);
}

void Commands::commitStagedChanges(const std::string& message, const std::string& secondParent) {
    std::vector<std::string> added = StagingArea::addedFiles();
    std::vector<std::string> removed = StagingArea::removedFiles();
    if (added.empty() && removed.empty()) {
        Utils::exitWithMessage("No changes added to the commit.");
    }

    // The new snapshot starts as a copy of the parent's, with staged
    // additions and removals applied on top.
    std::map<std::string, std::string> blobs = Repository::headCommit().getBlobs();
    for (const auto& f : removed) {
        blobs.erase(f);
    }
    for (const auto& f : added) {
        std::string contents = StagingArea::stagedContents(f);
        std::string blobId = Utils::sha1(contents);
        ObjectStore::writeBlob(blobId, contents);
        blobs[f] = blobId;
    }

    std::vector<std::string> parents{Repository::headCommitId()};
    if (!secondParent.empty()) {
        parents.push_back(secondParent);
    }
    Commit c(message, static_cast<long long>(std::time(nullptr)), parents, blobs);
    ObjectStore::writeCommit(c);
    Repository::setBranchHead(Repository::currentBranch(), c.id());
    StagingArea::clear();
}

void Commands::commit(const std::string& message) {
    if (message.empty()) {
        Utils::exitWithMessage("Please enter a commit message.");
    }
    commitStagedChanges(message, "");
}

void Commands::rm(const std::string& filename) {
    bool staged = StagingArea::isStagedForAddition(filename);
    bool tracked = Repository::headCommit().tracks(filename);
    if (!staged && !tracked) {
        Utils::exitWithMessage("No reason to remove the file.");
    }
    if (staged) {
        // Staged for addition but not yet committed: just unstage it
        // and leave the working file alone.
        StagingArea::unstage(filename);
    }
    if (tracked) {
        StagingArea::stageRemoval(filename);
        if (Utils::isFile(filename)) {
            Utils::restrictedDelete(filename);
        }
    }
}
