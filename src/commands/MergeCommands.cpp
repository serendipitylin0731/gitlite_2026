#include "../../include/commands/MergeCommands.h"
#include "../../include/commands/BasicCommands.h"
#include "../../include/commands/CheckoutCommands.h"
#include "../../include/Repository.h"
#include "../../include/ObjectStore.h"
#include "../../include/StagingArea.h"
#include "../../include/CommitGraph.h"
#include "../../include/Commit.h"
#include "../../include/IgnoreRules.h"
#include "../../include/Utils.h"

#include <map>
#include <set>
#include <string>

namespace {

/** Blob id of F in BLOBS, or "" if F is absent (deleted / untracked). */
std::string blobOf(const std::map<std::string, std::string>& blobs, const std::string& f) {
    auto it = blobs.find(f);
    return it == blobs.end() ? "" : it->second;
}

enum class Action { NONE, WRITE_GIVEN, REMOVE, CONFLICT };

/** The conflict-marker replacement text for a file changed in
 *  different ways; a deleted side counts as an empty file. */
std::string conflictContents(const std::string& currentBlob, const std::string& givenBlob) {
    std::string cur = currentBlob.empty() ? "" : ObjectStore::readBlob(currentBlob);
    std::string giv = givenBlob.empty() ? "" : ObjectStore::readBlob(givenBlob);
    if (!cur.empty() && cur.back() != '\n') {
        cur += '\n';
    }
    if (!giv.empty() && giv.back() != '\n') {
        giv += '\n';
    }
    return "<<<<<<< HEAD\n" + cur + "=======\n" + giv + ">>>>>>>\n";
}

} // namespace

void Commands::merge(const std::string& branchName) {
    if (!StagingArea::isEmpty()) {
        Utils::exitWithMessage("You have uncommitted changes.");
    }
    if (!Repository::branchExists(branchName)) {
        Utils::exitWithMessage("A branch with that name does not exist.");
    }
    if (branchName == Repository::currentBranch()) {
        Utils::exitWithMessage("Cannot merge a branch with itself.");
    }

    std::string currentId = Repository::headCommitId();
    std::string givenId = Repository::branchHead(branchName);
    std::string splitId = CommitGraph::splitPoint(currentId, givenId);

    if (splitId == givenId) {
        Utils::exitWithMessage("Given branch is an ancestor of the current branch.");
    }
    if (splitId == currentId) {
        // Fast-forward: the current branch simply advances to the given
        // branch's head and the working directory follows.
        Commit given = ObjectStore::readCommit(givenId);
        assertNoUntrackedInWay(given);
        checkoutAllFiles(given);
        Repository::setBranchHead(Repository::currentBranch(), givenId);
        StagingArea::clear();
        Utils::exitWithMessage("Current branch fast-forwarded.");
    }

    Commit split = ObjectStore::readCommit(splitId);
    Commit current = ObjectStore::readCommit(currentId);
    Commit given = ObjectStore::readCommit(givenId);

    // First pass over the union of the three snapshots: decide per file.
    std::set<std::string> files;
    for (const auto& e : split.getBlobs()) files.insert(e.first);
    for (const auto& e : current.getBlobs()) files.insert(e.first);
    for (const auto& e : given.getBlobs()) files.insert(e.first);

    std::map<std::string, Action> actions;
    for (const auto& f : files) {
        std::string s = blobOf(split.getBlobs(), f);
        std::string c = blobOf(current.getBlobs(), f);
        std::string g = blobOf(given.getBlobs(), f);
        if (c == s) {
            if (g.empty() && !s.empty()) {
                actions[f] = Action::REMOVE;      // removed in given only
            } else if (!g.empty() && g != s) {
                actions[f] = Action::WRITE_GIVEN; // changed/added in given only
            }
        } else if (g != s && c != g) {
            actions[f] = Action::CONFLICT;        // changed in different ways
        }
        // Remaining cases: unchanged in given, or changed identically in
        // both branches (including deleted in both) — nothing to do.
    }

    // The merge must not clobber an untracked working file.
    IgnoreRules ignoreRules;
    for (const auto& entry : actions) {
        if (entry.second == Action::WRITE_GIVEN || entry.second == Action::CONFLICT) {
            const std::string& f = entry.first;
            if (Utils::isFile(f) && !current.tracks(f) && !ignoreRules.ignores(f)) {
                Utils::exitWithMessage(
                    "There is an untracked file in the way; delete it, or add and commit it first.");
            }
        }
    }

    // Second pass: apply to the working directory and stage the results.
    bool conflicted = false;
    for (const auto& entry : actions) {
        const std::string& f = entry.first;
        switch (entry.second) {
        case Action::WRITE_GIVEN: {
            std::string contents = ObjectStore::readBlob(blobOf(given.getBlobs(), f));
            Utils::writeContents(f, contents);
            StagingArea::stageAddition(f, contents);
            break;
        }
        case Action::REMOVE:
            StagingArea::stageRemoval(f);
            if (Utils::isFile(f)) {
                Utils::restrictedDelete(f);
            }
            break;
        case Action::CONFLICT: {
            conflicted = true;
            std::string merged = conflictContents(blobOf(current.getBlobs(), f),
                                                  blobOf(given.getBlobs(), f));
            Utils::writeContents(f, merged);
            StagingArea::stageAddition(f, merged);
            break;
        }
        case Action::NONE:
            break;
        }
    }

    // A real merge always commits (conflicts included), recording the
    // given branch's head as the second parent.
    std::string message = "Merged " + branchName + " into " + Repository::currentBranch() + ".";
    Commands::commitStagedChanges(message, givenId);
    if (conflicted) {
        Utils::message("Encountered a merge conflict.");
    }
}
