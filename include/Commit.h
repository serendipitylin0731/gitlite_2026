#ifndef COMMIT_H
#define COMMIT_H

#include <string>
#include <vector>
#include <map>

/**
 * A single commit node in the Gitlite commit tree.
 *
 * A commit records a log message, a timestamp (seconds since the Unix
 * epoch), a list of parent commit ids (one for normal commits, two for
 * merge commits, none for the initial commit) and a snapshot of tracked
 * files mapping filename -> blob id (SHA-1 of the file contents).
 *
 * Commits are immutable once written and are identified by the SHA-1 of
 * their serialized form.
 */
class Commit {
public:
    Commit();
    Commit(const std::string& message, long long timestamp,
           const std::vector<std::string>& parents,
           const std::map<std::string, std::string>& blobs);

    /** The 40-char hex SHA-1 id of this commit. */
    std::string id() const;

    /** Textual, self-delimiting serialization stored in objects/commits. */
    std::string serialize() const;

    /** Rebuild a commit from its serialized form. */
    static Commit deserialize(const std::string& data);

    const std::string& getMessage() const;
    long long getTimestamp() const;
    const std::vector<std::string>& getParents() const;
    const std::map<std::string, std::string>& getBlobs() const;

    /** True if FILENAME is tracked by this commit. */
    bool tracks(const std::string& filename) const;

    /** Blob id of FILENAME, or "" if the file is not tracked. */
    std::string blobId(const std::string& filename) const;

    /** Timestamp rendered as "EEE MMM dd HH:mm:ss yyyy Z" in the local zone. */
    std::string dateString() const;

private:
    std::string message;
    long long timestamp;
    std::vector<std::string> parents;
    std::map<std::string, std::string> blobs; // filename -> blob id (sorted)
};

#endif // COMMIT_H
