#ifndef DIFF_COMMANDS_H
#define DIFF_COMMANDS_H

#include <string>

/**
 * Subtask 6 command: diff — line-based comparison of file contents.
 *
 * Three forms, mirroring checkout's multi-form dispatch. Only tracked
 * files are compared: untracked working files and the staging area do
 * not take part. Nothing is printed when there are no differences.
 */
namespace Commands {

void diff();                                               // 当前提交 vs 工作目录
void diffWithCommit(const std::string& commitId);          // 指定提交 vs 工作目录
void diffBetween(const std::string& commitId1,
                 const std::string& commitId2);            // 两个指定提交互比

}

#endif // DIFF_COMMANDS_H
