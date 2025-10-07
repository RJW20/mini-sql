#ifndef DIRECTORY_ITERABLE_HPP
#define DIRECTORY_ITERABLE_HPP

#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

/* Directory Iterable
 * Yeilds all files from a directory, or only those with certain prefixes.
 * Maintains order output to match that of the prefixes when given.*/
class DirectoryIterable {
public:
    DirectoryIterable(const fs::path& dir) {
        for (const auto& entry : fs::directory_iterator(dir))
            files_.push_back(entry.path());
    }

    DirectoryIterable(
        const fs::path& dir, const std::vector<std::string>& prefixes
    ) {
        std::unordered_map<std::string, std::vector<fs::path>> prefix_map;
        for (const auto& entry : fs::directory_iterator(dir)) {
            const std::string filename = entry.path().filename().string();
            for (const std::string& prefix : prefixes)
                if (filename.rfind(prefix, 0) == 0) {
                    prefix_map[prefix].push_back(entry.path());
                    break;
                }
        }
        for (const std::string& prefix : prefixes) {
            auto it = prefix_map.find(prefix);
            if (it != prefix_map.end())
                files_.insert(
                    files_.end(), it->second.begin(), it->second.end()
                );
        }
    }

    std::vector<fs::path>::const_iterator begin() const {
        return files_.begin();
    }
    std::vector<fs::path>::const_iterator end() const {
        return files_.end();
    }

private:
    std::vector<fs::path> files_;
};

#endif // DIRECTORY_ITERABLE_HPP