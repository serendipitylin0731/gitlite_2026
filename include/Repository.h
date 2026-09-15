#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <string>
#include <vector>
#include "Commit.h"

/**
 * Low-level access to the on-disk state of a Gitlite repository.
 *
 * On-disk layout (all under ./.gitlite):
 *   HEAD                    name of the current branch
 *   refs/heads/<branch>     branch pointers; nested dirs allow "R1/master"
 *   objects/commits/<sha1>  serialized Commit objects (see ObjectStore)
 *   objects/blobs/<sha1>    file contents addressed by content hash
 *   staging/added/<name>    files staged for addition (see StagingArea)
 *   staging/removed/<name>  files staged for removal (see StagingArea)
 *   remotes/<name>          path of a remote .gitlite directory
 *
 * This class owns only paths, the HEAD pointer and the branch refs;
 * object persistence lives in ObjectStore, the staging area in
 * StagingArea, and user-level commands in the Commands modules.
 */
class Repository {
public:
    /* Paths. */
    static std::string getGitliteDir();
    static std::string headFile();
    static std::string headsDir();
    static std::string objectsDir();
    static std::string commitsDir();
    static std::string blobsDir();
    static std::string addedDir();
    static std::string removedDir();
    static std::string remotesDir();
    static std::string branchFile(const std::string& name);

    /** Create the empty .gitlite directory skeleton (used by init). */
    static void createLayout();

    /* HEAD and branch refs. */
    static std::string currentBranch();
    static void setCurrentBranch(const std::string& name);
    static bool branchExists(const std::string& name);
    static std::vector<std::string> allBranches();
    static std::string branchHead(const std::string& name);
    static void setBranchHead(const std::string& name, const std::string& commitId);
    static void removeBranch(const std::string& name);
    static std::string headCommitId();
    static Commit headCommit();
};

#endif // REPOSITORY_H
