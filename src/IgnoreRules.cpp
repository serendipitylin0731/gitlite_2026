#include "../include/IgnoreRules.h"
#include "../include/Utils.h"

#include <sstream>

namespace {

enum class TokenKind { LITERAL, STAR, QUESTION };

struct Token {
    TokenKind kind;
    char literal = '\0';
};

std::vector<Token> tokenize(const std::string& pattern) {
    std::vector<Token> tokens;
    for (size_t i = 0; i < pattern.size(); ++i) {
        char ch = pattern[i];
        if (ch == '\\' && i + 1 < pattern.size()) {
            tokens.push_back({TokenKind::LITERAL, pattern[++i]});
        } else if (ch == '*') {
            // Adjacent stars have the same meaning as a single star.
            if (tokens.empty() || tokens.back().kind != TokenKind::STAR) {
                tokens.push_back({TokenKind::STAR});
            }
        } else if (ch == '?') {
            tokens.push_back({TokenKind::QUESTION});
        } else {
            tokens.push_back({TokenKind::LITERAL, ch});
        }
    }
    return tokens;
}

} // namespace

IgnoreRules::IgnoreRules() {
    const std::string ignoreFile = ".gitliteignore";
    if (!Utils::isFile(ignoreFile)) {
        return;
    }

    std::istringstream input(Utils::readContentsAsString(ignoreFile));
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#') {
            continue;
        }

        bool negated = false;
        if (line[0] == '!') {
            negated = true;
            line.erase(0, 1);
        }
        // A leading slash explicitly anchors a rule at the repository root.
        // Gitlite has a flat working tree, so anchored and unanchored rules
        // otherwise have identical matching behavior.
        if (!line.empty() && line[0] == '/') {
            line.erase(0, 1);
        }
        if (!line.empty()) {
            rules.push_back({line, negated});
        }
    }
}

bool IgnoreRules::ignores(const std::string& filename) const {
    bool ignored = false;
    for (const auto& rule : rules) {
        if (matches(rule.pattern, filename)) {
            ignored = !rule.negated;
        }
    }
    return ignored;
}

bool IgnoreRules::matches(const std::string& pattern, const std::string& filename) {
    std::vector<Token> tokens = tokenize(pattern);
    std::vector<bool> previous(filename.size() + 1, false);
    previous[0] = true;

    for (const auto& token : tokens) {
        std::vector<bool> current(filename.size() + 1, false);
        if (token.kind == TokenKind::STAR) {
            current[0] = previous[0];
            for (size_t i = 1; i <= filename.size(); ++i) {
                current[i] = previous[i] || current[i - 1];
            }
        } else {
            for (size_t i = 1; i <= filename.size(); ++i) {
                bool same = token.kind == TokenKind::QUESTION
                         || token.literal == filename[i - 1];
                current[i] = previous[i - 1] && same;
            }
        }
        previous.swap(current);
    }
    return previous[filename.size()];
}
