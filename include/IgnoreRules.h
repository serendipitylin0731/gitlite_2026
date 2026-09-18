#ifndef IGNORE_RULES_H
#define IGNORE_RULES_H

#include <string>
#include <vector>

/**
 * Rules loaded from .gitliteignore in the working directory.
 *
 * Gitlite only manages plain files in the repository root, so patterns are
 * matched against those file names.  '*' matches any byte sequence, '?'
 * matches one byte, a leading '#' starts a comment, and a leading '!'
 * negates a rule.  As in Git, the last matching rule wins.
 */
class IgnoreRules {
public:
    IgnoreRules();

    /** True when FILENAME is excluded by the loaded rules. */
    bool ignores(const std::string& filename) const;

private:
    struct Rule {
        std::string pattern;
        bool negated;
    };

    std::vector<Rule> rules;

    static bool matches(const std::string& pattern, const std::string& filename);
};

#endif // IGNORE_RULES_H
