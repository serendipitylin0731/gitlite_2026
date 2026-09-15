#ifndef STAGING_AREA_H
#define STAGING_AREA_H

#include <string>
#include <vector>

/**
 * The Gitlite staging area, persisted as plain files:
 *
 *   .gitlite/staging/added/<name>    snapshot of a file staged for addition
 *   .gitlite/staging/removed/<name>  empty marker for a file staged for removal
 *
 * Staging a file copies its contents at `add` time, so later edits or
 * deletions in the working directory do not affect the next commit.
 */
class StagingArea {
public:
    static std::vector<std::string> addedFiles();
    static std::vector<std::string> removedFiles();
    static bool isEmpty();

    static bool isStagedForAddition(const std::string& filename);
    static bool isStagedForRemoval(const std::string& filename);

    /** Stage CONTENTS as the next committed version of FILENAME and
     *  cancel any pending removal mark for it. */
    static void stageAddition(const std::string& filename, const std::string& contents);

    /** Mark FILENAME to be untracked by the next commit. */
    static void stageRemoval(const std::string& filename);

    /** Drop both the addition and the removal mark for FILENAME. */
    static void unstage(const std::string& filename);

    /** Contents of a file staged for addition. */
    static std::string stagedContents(const std::string& filename);

    static void clear();
};

#endif // STAGING_AREA_H
