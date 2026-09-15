#ifndef BASIC_COMMANDS_H
#define BASIC_COMMANDS_H

#include <string>

/**
 * Subtask 1 commands: init, add, commit, rm.
 */
namespace Commands {

void init();
void add(const std::string& filename);
void commit(const std::string& message);
void rm(const std::string& filename);

/** Persist the current staging area as a new commit on the current
 *  branch (shared with merge, which passes a second parent). Exits
 *  with "No changes added to the commit." when the staging area and
 *  the removal list are both empty. */
void commitStagedChanges(const std::string& message, const std::string& secondParent);

}

#endif // BASIC_COMMANDS_H
