#include "../../include/commands/HistoryCommands.h"
#include "../../include/Repository.h"
#include "../../include/ObjectStore.h"
#include "../../include/Commit.h"
#include "../../include/Utils.h"

#include <iostream>

/** Print one log entry: "===" header, id, optional Merge line for
 *  two-parent commits, date, message, then a blank line. */
static void printEntry(const Commit& c) {
    std::cout << "===" << std::endl;
    std::cout << "commit " << c.id() << std::endl;
    if (c.getParents().size() == 2) {
        std::cout << "Merge: " << c.getParents()[0].substr(0, 7)
                  << " " << c.getParents()[1].substr(0, 7) << std::endl;
    }
    std::cout << "Date: " << c.dateString() << std::endl;
    std::cout << c.getMessage() << std::endl;
    std::cout << std::endl;
}

void Commands::log() {
    // Walk backwards from the head following first parents only.
    std::string id = Repository::headCommitId();
    while (!id.empty()) {
        Commit c = ObjectStore::readCommit(id);
        printEntry(c);
        id = c.getParents().empty() ? "" : c.getParents()[0];
    }
}

void Commands::globalLog() {
    // Every commit ever created, reachable or not; order is irrelevant.
    for (const auto& id : ObjectStore::allCommitIds()) {
        printEntry(ObjectStore::readCommit(id));
    }
}

void Commands::find(const std::string& message) {
    bool found = false;
    for (const auto& id : ObjectStore::allCommitIds()) {
        if (ObjectStore::readCommit(id).getMessage() == message) {
            std::cout << id << std::endl;
            found = true;
        }
    }
    if (!found) {
        Utils::exitWithMessage("Found no commit with that message.");
    }
}
