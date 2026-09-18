#include "../../include/commands/CheckoutCommands.h"
#include "../../include/Repository.h"
#include "../../include/ObjectStore.h"
#include "../../include/StagingArea.h"
#include "../../include/Commit.h"
#include "../../include/IgnoreRules.h"
#include "../../include/Utils.h"

void Commands::checkoutAllFiles(const Commit& target) {
    Commit head = Repository::headCommit();
    for (const auto& entry : head.getBlobs()) {
        if (!target.tracks(entry.first)) {
            Utils::restrictedDelete(entry.first);
        }
    }
    for (const auto& entry : target.getBlobs()) {
        Utils::writeContents(entry.first, ObjectStore::readBlob(entry.second));
    }
}

void Commands::assertNoUntrackedInWay(const Commit& target) {
    Commit head = Repository::headCommit();
    IgnoreRules ignoreRules;
    for (const auto& entry : target.getBlobs()) {
        const std::string& f = entry.first;
        if (Utils::isFile(f) && !head.tracks(f)
                && !StagingArea::isStagedForAddition(f)
                && !ignoreRules.ignores(f)) {
            Utils::exitWithMessage(
                "There is an untracked file in the way; delete it, or add and commit it first.");
        }
    }
}

void Commands::checkoutFile(const std::string& filename) {
    Commit head = Repository::headCommit();
    if (!head.tracks(filename)) {
        Utils::exitWithMessage("File does not exist in that commit.");
    }
    Utils::writeContents(filename, ObjectStore::readBlob(head.blobId(filename)));
}

void Commands::checkoutFileInCommit(const std::string& commitId, const std::string& filename) {
    std::string resolved = ObjectStore::resolveCommitId(commitId);
    if (resolved.empty()) {
        Utils::exitWithMessage("No commit with that id exists.");
    }
    Commit c = ObjectStore::readCommit(resolved);
    if (!c.tracks(filename)) {
        Utils::exitWithMessage("File does not exist in that commit.");
    }
    Utils::writeContents(filename, ObjectStore::readBlob(c.blobId(filename)));
}

void Commands::checkoutBranch(const std::string& branchName) {
    if (!Repository::branchExists(branchName)) {
        Utils::exitWithMessage("No such branch exists.");
    }
    if (branchName == Repository::currentBranch()) {
        Utils::exitWithMessage("No need to checkout the current branch.");
    }
    Commit target = ObjectStore::readCommit(Repository::branchHead(branchName));
    assertNoUntrackedInWay(target);
    checkoutAllFiles(target);
    StagingArea::clear();
    Repository::setCurrentBranch(branchName);
}

void Commands::reset(const std::string& commitId) {
    std::string resolved = ObjectStore::resolveCommitId(commitId);
    if (resolved.empty()) {
        Utils::exitWithMessage("No commit with that id exists.");
    }
    Commit target = ObjectStore::readCommit(resolved);
    assertNoUntrackedInWay(target);
    checkoutAllFiles(target);
    Repository::setBranchHead(Repository::currentBranch(), resolved);
    StagingArea::clear();
}
