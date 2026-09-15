#include "../include/StagingArea.h"
#include "../include/Repository.h"
#include "../include/Utils.h"

#include <cstdio>

std::vector<std::string> StagingArea::addedFiles() {
    return Utils::plainFilenamesIn(Repository::addedDir());
}

std::vector<std::string> StagingArea::removedFiles() {
    return Utils::plainFilenamesIn(Repository::removedDir());
}

bool StagingArea::isEmpty() {
    return addedFiles().empty() && removedFiles().empty();
}

bool StagingArea::isStagedForAddition(const std::string& filename) {
    return Utils::isFile(Utils::join(Repository::addedDir(), filename));
}

bool StagingArea::isStagedForRemoval(const std::string& filename) {
    return Utils::isFile(Utils::join(Repository::removedDir(), filename));
}

void StagingArea::stageAddition(const std::string& filename, const std::string& contents) {
    Utils::writeContents(Utils::join(Repository::addedDir(), filename), contents);
    std::string removalMark = Utils::join(Repository::removedDir(), filename);
    if (Utils::isFile(removalMark)) {
        remove(removalMark.c_str());
    }
}

void StagingArea::stageRemoval(const std::string& filename) {
    Utils::writeContents(Utils::join(Repository::removedDir(), filename), "");
}

void StagingArea::unstage(const std::string& filename) {
    std::string added = Utils::join(Repository::addedDir(), filename);
    if (Utils::isFile(added)) {
        remove(added.c_str());
    }
    std::string removed = Utils::join(Repository::removedDir(), filename);
    if (Utils::isFile(removed)) {
        remove(removed.c_str());
    }
}

std::string StagingArea::stagedContents(const std::string& filename) {
    return Utils::readContentsAsString(Utils::join(Repository::addedDir(), filename));
}

void StagingArea::clear() {
    for (const auto& f : addedFiles()) {
        remove(Utils::join(Repository::addedDir(), f).c_str());
    }
    for (const auto& f : removedFiles()) {
        remove(Utils::join(Repository::removedDir(), f).c_str());
    }
}
