#ifndef HISTORY_COMMANDS_H
#define HISTORY_COMMANDS_H

#include <string>

/**
 * Subtask 2 history commands: log, global-log, find.
 */
namespace Commands {

void log();
void globalLog();
void find(const std::string& message);

}

#endif // HISTORY_COMMANDS_H
