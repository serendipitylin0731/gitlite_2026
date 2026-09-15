#include "../include/ObjectStore.h"
#include "../include/Repository.h"
#include "../include/Utils.h"

bool ObjectStore::commitExists(const std::string& id) {
    return Utils::isFile(Utils::join(Repository::commitsDir(), id));
}

Commit ObjectStore::readCommit(const std::string& id) {
    return Commit::deserialize(
        Utils::readContentsAsString(Utils::join(Repository::commitsDir(), id)));
}

void ObjectStore::writeCommit(const Commit& c) {
    Utils::writeContents(Utils::join(Repository::commitsDir(), c.id()), c.serialize());
}

std::vector<std::string> ObjectStore::allCommitIds() {
    return Utils::plainFilenamesIn(Repository::commitsDir());
}

std::string ObjectStore::resolveCommitId(const std::string& idOrPrefix) {
    if (idOrPrefix.size() == Utils::UID_LENGTH && commitExists(idOrPrefix)) {
        return idOrPrefix;
    }
    for (const auto& candidate : allCommitIds()) {
        if (candidate.compare(0, idOrPrefix.size(), idOrPrefix) == 0) {
            return candidate;
        }
    }
    return "";
}

std::string ObjectStore::readBlob(const std::string& id) {
    return Utils::readContentsAsString(Utils::join(Repository::blobsDir(), id));
}

void ObjectStore::writeBlob(const std::string& id, const std::string& contents) {
    std::string file = Utils::join(Repository::blobsDir(), id);
    if (!Utils::isFile(file)) {
        Utils::writeContents(file, contents);
    }
}

void ObjectStore::copyObjects(const std::string& srcGitlite, const std::string& dstGitlite) {
    const char* kinds[] = {"commits", "blobs"};
    for (const char* kind : kinds) {
        std::string srcDir = Utils::join(srcGitlite, "objects", kind);
        std::string dstDir = Utils::join(dstGitlite, "objects", kind);
        for (const auto& f : Utils::plainFilenamesIn(srcDir)) {
            std::string dst = Utils::join(dstDir, f);
            if (!Utils::exists(dst)) {
                Utils::writeContents(dst, Utils::readContents(Utils::join(srcDir, f)));
            }
        }
    }
}
