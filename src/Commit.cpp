#include "../include/Commit.h"
#include "../include/Utils.h"

#include <ctime>
#include <stdexcept>

Commit::Commit() : timestamp(0) {}

Commit::Commit(const std::string& message, long long timestamp,
               const std::vector<std::string>& parents,
               const std::map<std::string, std::string>& blobs)
    : message(message), timestamp(timestamp), parents(parents), blobs(blobs) {}

std::string Commit::id() const {
    return Utils::sha1(serialize());
}

std::string Commit::serialize() const {
    std::string out;
    out += std::to_string(message.size()) + "\n";
    out += message;
    out += std::to_string(timestamp) + "\n";
    out += std::to_string(parents.size()) + "\n";
    for (const auto& p : parents) {
        out += p + "\n";
    }
    out += std::to_string(blobs.size()) + "\n";
    for (const auto& entry : blobs) {
        out += std::to_string(entry.first.size()) + " " + entry.first
             + entry.second + "\n";
    }
    return out;
}

Commit Commit::deserialize(const std::string& data) {
    size_t pos = 0;
    auto readLine = [&]() -> std::string {
        size_t nl = data.find('\n', pos);
        if (nl == std::string::npos) {
            throw std::runtime_error("corrupt commit object");
        }
        std::string line = data.substr(pos, nl - pos);
        pos = nl + 1;
        return line;
    };

    Commit c;
    size_t msgLen = std::stoul(readLine());
    c.message = data.substr(pos, msgLen);
    pos += msgLen;
    c.timestamp = std::stoll(readLine());
    size_t numParents = std::stoul(readLine());
    for (size_t i = 0; i < numParents; ++i) {
        c.parents.push_back(readLine());
    }
    size_t numBlobs = std::stoul(readLine());
    for (size_t i = 0; i < numBlobs; ++i) {
        std::string header = readLine();
        size_t space = header.find(' ');
        size_t nameLen = std::stoul(header.substr(0, space));
        std::string name = header.substr(space + 1, nameLen);
        std::string blob = header.substr(space + 1 + nameLen);
        c.blobs[name] = blob;
    }
    return c;
}

const std::string& Commit::getMessage() const {
    return message;
}

long long Commit::getTimestamp() const {
    return timestamp;
}

const std::vector<std::string>& Commit::getParents() const {
    return parents;
}

const std::map<std::string, std::string>& Commit::getBlobs() const {
    return blobs;
}

bool Commit::tracks(const std::string& filename) const {
    return blobs.find(filename) != blobs.end();
}

std::string Commit::blobId(const std::string& filename) const {
    auto it = blobs.find(filename);
    return it == blobs.end() ? "" : it->second;
}

std::string Commit::dateString() const {
    std::time_t t = static_cast<std::time_t>(timestamp);
    std::tm tmBuf;
    localtime_r(&t, &tmBuf);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y %z", &tmBuf);
    return std::string(buf);
}
