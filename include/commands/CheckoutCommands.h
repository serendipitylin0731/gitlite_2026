#ifndef CHECKOUT_COMMANDS_H
#define CHECKOUT_COMMANDS_H

#include <string>
#include "../Commit.h"

/**
 * Subtask 2/3/4 checkout-family commands: the three checkout forms
 * plus reset ("checkout of an arbitrary commit that also moves the
 * current branch head").
 */
namespace Commands {

void checkoutFile(const std::string& filename);
void checkoutFileInCommit(const std::string& commitId, const std::string& filename);
void checkoutBranch(const std::string& branchName);
void reset(const std::string& commitId);

/** Overwrite the working directory with TARGET's files and delete
 *  files tracked by the current head but absent from TARGET. Callers
 *  must have run assertNoUntrackedInWay first. */
void checkoutAllFiles(const Commit& target);

/** Exit with an error if a working-directory file not tracked by the
 *  current head would be overwritten by checking out TARGET. */
void assertNoUntrackedInWay(const Commit& target);

}

#endif // CHECKOUT_COMMANDS_H
