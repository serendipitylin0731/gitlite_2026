#ifndef COMMIT_GRAPH_H
#define COMMIT_GRAPH_H

#include <string>
#include <set>

/**
 * Read-only traversals of the commit DAG. Both parents of merge
 * commits are followed, so the history behind a merged-in branch is
 * fully taken into account.
 */
namespace CommitGraph {

/** Ids of every commit reachable from COMMITID, inclusive. */
std::set<std::string> ancestorsOf(const std::string& commitId);

/** True if ANCESTORID lies in the history of DESCENDANTID (inclusive). */
bool isAncestor(const std::string& ancestorId, const std::string& descendantId);

/** The split point (lowest common ancestor) of two commits: BFS outward
 *  from GIVENID, the first commit also reachable from CURRENTID wins. */
std::string splitPoint(const std::string& currentId, const std::string& givenId);

}

#endif // COMMIT_GRAPH_H
