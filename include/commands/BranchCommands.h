#ifndef BRANCH_COMMANDS_H
#define BRANCH_COMMANDS_H

#include <string>

/**
 * Subtask 4 branch commands: branch, rm-branch.
 */
namespace Commands {

void branch(const std::string& name);
void rmBranch(const std::string& name);

}

#endif // BRANCH_COMMANDS_H
