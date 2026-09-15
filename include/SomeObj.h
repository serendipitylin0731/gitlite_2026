#ifndef SOME_OBJ_H
#define SOME_OBJ_H

#include <string>

/**
 * Command facade used by main.cpp (which is fixed and must not change).
 *
 * Every method simply forwards to the corresponding function of the
 * command modules under include/commands/; the object carries no state
 * of its own because each gitlite invocation is a fresh process whose
 * entire state lives in the .gitlite directory.
 */
class SomeObj {
public:
    SomeObj();

    void init();
    void addRemote(const std::string& name, const std::string& path);
    void rmRemote(const std::string& name);
    void add(const std::string& filename);
    void commit(const std::string& message);
    void rm(const std::string& filename);
    void log();
    void globalLog();
    void find(const std::string& message);
    void status();
    void checkoutBranch(const std::string& branchName);
    void checkoutFile(const std::string& filename);
    void checkoutFileInCommit(const std::string& commitId, const std::string& filename);
    void branch(const std::string& name);
    void rmBranch(const std::string& name);
    void reset(const std::string& commitId);
    void merge(const std::string& branchName);
    void push(const std::string& remoteName, const std::string& branchName);
    void fetch(const std::string& remoteName, const std::string& branchName);
    void pull(const std::string& remoteName, const std::string& branchName);
};

#endif // SOME_OBJ_H
