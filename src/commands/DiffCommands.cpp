#include "../../include/commands/DiffCommands.h"
#include "../../include/Repository.h"
#include "../../include/ObjectStore.h"
#include "../../include/Commit.h"
#include "../../include/Differ.h"
#include "../../include/Utils.h"

#include <iostream>
#include <set>

namespace {

/** Unified diff for one file, including the diff/---/+++ header
 *  lines; a non-existing side is shown as /dev/null. */
std::string fileDiff(const std::string& name, bool oldExists, const std::string& oldContent,
                     bool newExists, const std::string& newContent) {
    std::string out = "diff --gitlite a/" + name + " b/" + name + "\n";
    out += "--- " + (oldExists ? "a/" + name : std::string("/dev/null")) + "\n";
    out += "+++ " + (newExists ? "b/" + name : std::string("/dev/null")) + "\n";
    out += Differ::unified(oldContent, newContent);
    return out;
}

/** Compare a commit snapshot with the working directory. */
void diffAgainstWorkdir(const Commit& base) {
    for (const auto& entry : base.getBlobs()) {
        const std::string& name = entry.first;
        if (!Utils::isFile(name)) {
            std::cout << fileDiff(name, true, ObjectStore::readBlob(entry.second), false, "");
            continue;
        }
        std::string work = Utils::readContentsAsString(name);
        if (Utils::sha1(work) != entry.second) {
            std::cout << fileDiff(name, true, ObjectStore::readBlob(entry.second), true, work);
        }
    }
}

/** Resolve ID or exit with the standard message. */
Commit requireCommit(const std::string& commitId) {
    std::string resolved = ObjectStore::resolveCommitId(commitId);
    if (resolved.empty()) {
        Utils::exitWithMessage("No commit with that id exists.");
    }
    return ObjectStore::readCommit(resolved);
}

} // namespace

void Commands::diff() {
    diffAgainstWorkdir(Repository::headCommit());
}

void Commands::diffWithCommit(const std::string& commitId) {
    diffAgainstWorkdir(requireCommit(commitId));
}

void Commands::diffBetween(const std::string& commitId1, const std::string& commitId2) {
    Commit oldCommit = requireCommit(commitId1);
    Commit newCommit = requireCommit(commitId2);

    std::set<std::string> names;
    for (const auto& e : oldCommit.getBlobs()) names.insert(e.first);
    for (const auto& e : newCommit.getBlobs()) names.insert(e.first);

    for (const auto& name : names) {
        std::string oldBlob = oldCommit.blobId(name);
        std::string newBlob = newCommit.blobId(name);
        if (oldBlob == newBlob) {
            continue; // identical on both sides (or absent from both)
        }
        bool oldExists = !oldBlob.empty();
        bool newExists = !newBlob.empty();
        std::cout << fileDiff(name, oldExists, oldExists ? ObjectStore::readBlob(oldBlob) : "",
                              newExists, newExists ? ObjectStore::readBlob(newBlob) : "");
    }
}
