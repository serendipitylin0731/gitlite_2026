#ifndef REMOTE_COMMANDS_H
#define REMOTE_COMMANDS_H

#include <string>

/**
 * Subtask 6 remote commands: add-remote, rm-remote, push, fetch, pull.
 *
 * A remote is simply another local .gitlite directory whose path is
 * stored under .gitlite/remotes/<name>; fetched branches are tracked
 * locally under the name "<remote>/<branch>".
 */
namespace Commands {

void addRemote(const std::string& name, const std::string& path);
void rmRemote(const std::string& name);
void push(const std::string& remoteName, const std::string& branchName);
void fetch(const std::string& remoteName, const std::string& branchName);
void pull(const std::string& remoteName, const std::string& branchName);

}

#endif // REMOTE_COMMANDS_H
