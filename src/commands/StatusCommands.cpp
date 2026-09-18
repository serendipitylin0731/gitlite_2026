#include "../../include/commands/StatusCommands.h"
#include "../../include/Repository.h"
#include "../../include/StagingArea.h"
#include "../../include/Commit.h"
#include "../../include/IgnoreRules.h"
#include "../../include/Utils.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

void Commands::status() {
    Commit head = Repository::headCommit();
    std::vector<std::string> added = StagingArea::addedFiles();
    std::vector<std::string> removed = StagingArea::removedFiles();
    std::set<std::string> addedSet(added.begin(), added.end());
    std::set<std::string> removedSet(removed.begin(), removed.end());
    IgnoreRules ignoreRules;

    std::ostringstream out;
    out << "=== Branches ===\n";
    std::string current = Repository::currentBranch();
    for (const auto& b : Repository::allBranches()) {
        out << (b == current ? "*" : "") << b << "\n";
    }
    out << "\n=== Staged Files ===\n";
    for (const auto& f : added) {
        out << f << "\n";
    }
    out << "\n=== Removed Files ===\n";
    for (const auto& f : removed) {
        out << f << "\n";
    }

    std::vector<std::string> workFiles = Utils::plainFilenamesIn(".");
    std::set<std::string> workSet(workFiles.begin(), workFiles.end());

    // Modifications not staged for commit.
    std::vector<std::string> modifications;
    for (const auto& f : added) {
        if (workSet.count(f) == 0) {
            modifications.push_back(f + " (deleted)");
        } else if (StagingArea::stagedContents(f) != Utils::readContentsAsString(f)) {
            modifications.push_back(f + " (modified)");
        }
    }
    for (const auto& entry : head.getBlobs()) {
        const std::string& f = entry.first;
        if (addedSet.count(f) || removedSet.count(f)) {
            continue;
        }
        if (workSet.count(f) == 0) {
            modifications.push_back(f + " (deleted)");
        } else if (Utils::sha1(Utils::readContentsAsString(f)) != entry.second) {
            modifications.push_back(f + " (modified)");
        }
    }
    std::sort(modifications.begin(), modifications.end());
    out << "\n=== Modifications Not Staged For Commit ===\n";
    for (const auto& m : modifications) {
        out << m << "\n";
    }

    // Untracked files: in the working directory but neither staged for
    // addition nor tracked by the head commit. Files staged for removal
    // but recreated without Gitlite's knowledge count as untracked.
    out << "\n=== Untracked Files ===\n";
    for (const auto& f : workFiles) {
        if (addedSet.count(f)) {
            continue;
        }
        bool untracked = !head.tracks(f) || removedSet.count(f);
        bool mayBeIgnored = !head.tracks(f) && !addedSet.count(f);
        if (untracked && !(mayBeIgnored && ignoreRules.ignores(f))) {
            out << f << "\n";
        }
    }
    std::cout << out.str();
}
