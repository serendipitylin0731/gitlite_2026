#ifndef OBJECT_STORE_H
#define OBJECT_STORE_H

#include <string>
#include <vector>
#include "Commit.h"

/**
 * Content-addressed persistence for commits and blobs.
 *
 * Commits are stored in .gitlite/objects/commits/<sha1> in Commit's
 * serialized form; blobs are raw file contents in
 * .gitlite/objects/blobs/<sha1>, where the id is the SHA-1 of the
 * contents. Objects are immutable: writing an existing id is a no-op.
 */
class ObjectStore {
public:
    static bool commitExists(const std::string& id);
    static Commit readCommit(const std::string& id);
    static void writeCommit(const Commit& c);
    static std::vector<std::string> allCommitIds();

    /** Resolve a full or abbreviated (unique-prefix) id to an existing
     *  commit id; returns "" when no commit matches. */
    static std::string resolveCommitId(const std::string& idOrPrefix);

    static std::string readBlob(const std::string& id);
    static void writeBlob(const std::string& id, const std::string& contents);

    /** Copy every commit and blob object present in SRCGITLITE but
     *  missing from DSTGITLITE (both are .gitlite directories). */
    static void copyObjects(const std::string& srcGitlite, const std::string& dstGitlite);
};

#endif // OBJECT_STORE_H
