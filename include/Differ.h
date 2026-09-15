#ifndef DIFFER_H
#define DIFFER_H

#include <string>

/**
 * Line-based text differ: computes the unified-format difference
 * between two texts (LCS dynamic programming, context of 3 lines).
 * Text is compared byte-wise per line; a line keeps its line ending,
 * so CRLF contents round-trip unchanged.
 */
namespace Differ {

/** Unified-format hunks for OLD -> NEW (without the ---/+++ file
 *  headers); empty string when the two texts are identical. Lines
 *  lacking a trailing newline are followed by a
 *  "\ No newline at end of file" marker line. */
std::string unified(const std::string& oldText, const std::string& newText);

}

#endif // DIFFER_H
