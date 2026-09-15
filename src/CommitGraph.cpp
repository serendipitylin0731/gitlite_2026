#include "../include/CommitGraph.h"
#include "../include/ObjectStore.h"
#include "../include/Commit.h"

#include <queue>

std::set<std::string> CommitGraph::ancestorsOf(const std::string& commitId) {
    std::set<std::string> result;
    std::queue<std::string> q;
    q.push(commitId);
    while (!q.empty()) {
        std::string id = q.front();
        q.pop();
        if (!result.insert(id).second) {
            continue;
        }
        Commit c = ObjectStore::readCommit(id);
        for (const auto& p : c.getParents()) {
            q.push(p);
        }
    }
    return result;
}

bool CommitGraph::isAncestor(const std::string& ancestorId, const std::string& descendantId) {
    return ancestorsOf(descendantId).count(ancestorId) > 0;
}

std::string CommitGraph::splitPoint(const std::string& currentId, const std::string& givenId) {
    std::set<std::string> currentAncestors = ancestorsOf(currentId);
    std::set<std::string> visited;
    std::queue<std::string> q;
    q.push(givenId);
    while (!q.empty()) {
        std::string id = q.front();
        q.pop();
        if (!visited.insert(id).second) {
            continue;
        }
        if (currentAncestors.count(id)) {
            return id;
        }
        Commit c = ObjectStore::readCommit(id);
        for (const auto& p : c.getParents()) {
            q.push(p);
        }
    }
    return "";
}
