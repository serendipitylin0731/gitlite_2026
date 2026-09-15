#include "../include/SomeObj.h"
#include "../include/commands/BasicCommands.h"
#include "../include/commands/HistoryCommands.h"
#include "../include/commands/StatusCommands.h"
#include "../include/commands/CheckoutCommands.h"
#include "../include/commands/BranchCommands.h"
#include "../include/commands/MergeCommands.h"
#include "../include/commands/RemoteCommands.h"
#include "../include/commands/DiffCommands.h"

SomeObj::SomeObj() {}

void SomeObj::init() {
    Commands::init();
}

void SomeObj::addRemote(const std::string& name, const std::string& path) {
    Commands::addRemote(name, path);
}

void SomeObj::rmRemote(const std::string& name) {
    Commands::rmRemote(name);
}

void SomeObj::add(const std::string& filename) {
    Commands::add(filename);
}

void SomeObj::commit(const std::string& message) {
    Commands::commit(message);
}

void SomeObj::rm(const std::string& filename) {
    Commands::rm(filename);
}

void SomeObj::log() {
    Commands::log();
}

void SomeObj::globalLog() {
    Commands::globalLog();
}

void SomeObj::find(const std::string& message) {
    Commands::find(message);
}

void SomeObj::status() {
    Commands::status();
}

void SomeObj::checkoutBranch(const std::string& branchName) {
    Commands::checkoutBranch(branchName);
}

void SomeObj::checkoutFile(const std::string& filename) {
    Commands::checkoutFile(filename);
}

void SomeObj::checkoutFileInCommit(const std::string& commitId, const std::string& filename) {
    Commands::checkoutFileInCommit(commitId, filename);
}

void SomeObj::branch(const std::string& name) {
    Commands::branch(name);
}

void SomeObj::rmBranch(const std::string& name) {
    Commands::rmBranch(name);
}

void SomeObj::reset(const std::string& commitId) {
    Commands::reset(commitId);
}

void SomeObj::merge(const std::string& branchName) {
    Commands::merge(branchName);
}

void SomeObj::push(const std::string& remoteName, const std::string& branchName) {
    Commands::push(remoteName, branchName);
}

void SomeObj::fetch(const std::string& remoteName, const std::string& branchName) {
    Commands::fetch(remoteName, branchName);
}

void SomeObj::pull(const std::string& remoteName, const std::string& branchName) {
    Commands::pull(remoteName, branchName);
}

void SomeObj::diff() {
    Commands::diff();
}

void SomeObj::diffWithCommit(const std::string& commitId) {
    Commands::diffWithCommit(commitId);
}

void SomeObj::diffBetween(const std::string& commitId1, const std::string& commitId2) {
    Commands::diffBetween(commitId1, commitId2);
}
