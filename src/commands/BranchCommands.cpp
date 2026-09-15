#include "../../include/commands/BranchCommands.h"
#include "../../include/Repository.h"
#include "../../include/Utils.h"

void Commands::branch(const std::string& name) {
    if (Repository::branchExists(name)) {
        Utils::exitWithMessage("A branch with that name already exists.");
    }
    // A branch is just a new pointer at the current head commit; the
    // command does not switch to it.
    Repository::setBranchHead(name, Repository::headCommitId());
}

void Commands::rmBranch(const std::string& name) {
    if (!Repository::branchExists(name)) {
        Utils::exitWithMessage("A branch with that name does not exist.");
    }
    if (name == Repository::currentBranch()) {
        Utils::exitWithMessage("Cannot remove the current branch.");
    }
    // Only the pointer is removed; the commits stay in the object store.
    Repository::removeBranch(name);
}
