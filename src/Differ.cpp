#include "../include/Differ.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

const int CONTEXT = 3; // unchanged lines shown around each change

/** Split TEXT into lines; each line keeps its trailing '\n' (the last
 *  line may lack one). */
std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    size_t pos = 0;
    while (pos < text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string::npos) {
            lines.push_back(text.substr(pos));
            break;
        }
        lines.push_back(text.substr(pos, nl - pos + 1));
        pos = nl + 1;
    }
    return lines;
}

enum class Op { KEEP, DEL, ADD };

struct Edit {
    Op op;
    std::string line;
};

/** Longest-common-subsequence edit script turning OLD into NEW. */
std::vector<Edit> editScript(const std::vector<std::string>& oldLines,
                             const std::vector<std::string>& newLines) {
    size_t n = oldLines.size(), m = newLines.size();
    // dp[i][j] = LCS length of oldLines[i..] and newLines[j..].
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));
    for (size_t i = n; i-- > 0;) {
        for (size_t j = m; j-- > 0;) {
            if (oldLines[i] == newLines[j]) {
                dp[i][j] = dp[i + 1][j] + 1;
            } else {
                dp[i][j] = std::max(dp[i + 1][j], dp[i][j + 1]);
            }
        }
    }
    std::vector<Edit> ops;
    size_t i = 0, j = 0;
    while (i < n && j < m) {
        if (oldLines[i] == newLines[j]) {
            ops.push_back({Op::KEEP, oldLines[i]});
            ++i;
            ++j;
        } else if (dp[i + 1][j] >= dp[i][j + 1]) {
            ops.push_back({Op::DEL, oldLines[i]});
            ++i;
        } else {
            ops.push_back({Op::ADD, newLines[j]});
            ++j;
        }
    }
    while (i < n) ops.push_back({Op::DEL, oldLines[i++]});
    while (j < m) ops.push_back({Op::ADD, newLines[j++]});
    return ops;
}

bool isChange(const Edit& e) {
    return e.op != Op::KEEP;
}

void appendLine(std::string& out, char prefix, const std::string& line) {
    out += prefix;
    out += line;
    if (line.empty() || line.back() != '\n') {
        out += "\n\\ No newline at end of file\n";
    }
}

} // namespace

std::string Differ::unified(const std::string& oldText, const std::string& newText) {
    std::vector<std::string> oldLines = splitLines(oldText);
    std::vector<std::string> newLines = splitLines(newText);
    std::vector<Edit> ops = editScript(oldLines, newLines);

    std::string out;
    size_t i = 0;
    while (i < ops.size()) {
        // Locate the next hunk: a change plus CONTEXT lines around it;
        // changes separated by at most 2*CONTEXT keep-lines merge.
        size_t change = i;
        while (change < ops.size() && !isChange(ops[change])) {
            ++change;
        }
        if (change == ops.size()) {
            break;
        }
        size_t hunkBegin = change > static_cast<size_t>(CONTEXT) ? change - CONTEXT : 0;
        size_t hunkEnd = change + 1;
        for (size_t k = change + 1; k < ops.size() && k <= hunkEnd + 2 * CONTEXT; ++k) {
            if (isChange(ops[k])) {
                hunkEnd = k + 1;
            }
        }
        hunkEnd = std::min(ops.size(), hunkEnd + CONTEXT);

        // Line counts on each side within the hunk, and the number of
        // lines of each side before it (for the @@ header; an empty
        // side starts at 0, matching git's convention).
        size_t oldBefore = 0, newBefore = 0;
        for (size_t k = 0; k < hunkBegin; ++k) {
            if (ops[k].op != Op::ADD) ++oldBefore;
            if (ops[k].op != Op::DEL) ++newBefore;
        }
        size_t oldCount = 0, newCount = 0;
        for (size_t k = hunkBegin; k < hunkEnd; ++k) {
            if (ops[k].op != Op::ADD) ++oldCount;
            if (ops[k].op != Op::DEL) ++newCount;
        }
        size_t oldStart = oldCount == 0 ? oldBefore : oldBefore + 1;
        size_t newStart = newCount == 0 ? newBefore : newBefore + 1;

        out += "@@ -" + std::to_string(oldStart) + "," + std::to_string(oldCount)
             + " +" + std::to_string(newStart) + "," + std::to_string(newCount) + " @@\n";
        for (size_t k = hunkBegin; k < hunkEnd; ++k) {
            char prefix = ops[k].op == Op::KEEP ? ' ' : (ops[k].op == Op::DEL ? '-' : '+');
            appendLine(out, prefix, ops[k].line);
        }
        i = hunkEnd;
    }
    return out;
}
