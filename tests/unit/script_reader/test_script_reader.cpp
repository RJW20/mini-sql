#include "script_reader.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

bool round_trip_test(const fs::path& filename) {

    // Read original file
    std::ifstream script{filename};
    if (!script.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }
    minisql::ScriptReader reader_1{script};
    std::vector<std::string> statements_1;
    while (auto statement = reader_1.next())
        statements_1.push_back(*statement);

    // Second pass on first output
    std::ostringstream output;
    for (const std::string& s : statements_1) output << s << '\n';
    std::istringstream output_script{output.str()};
    minisql::ScriptReader reader_2{output_script};
    std::vector<std::string> statements_2;
    while (auto statement = reader_2.next())
        statements_2.push_back(*statement);

    // Compare outputs
    if (statements_1.size() != statements_2.size()) {
        std::cerr << "FAIL: " << filename.filename()
            << " — statement count mismatch " << statements_1.size() << " vs "
            << statements_2.size() << "\n";
        return false;
    }
    for (std::size_t i = 0; i < statements_1.size(); i++) {
        if (statements_1[i] != statements_2[i]) {
            std::cerr << "  FAIL: " << filename.filename()
                << " — mismatch at statement " << i+1 << "\n";
            std::cerr << "    First pass:  [" << statements_1[i] << "]\n";
            std::cerr << "    Second pass: [" << statements_2[i] << "]\n";
            return false;
        }
    }

    std::cout << "  PASS: " << filename.filename() << " ("
        << statements_1.size() << " statement"
        << ((statements_1.size() > 1) ? "s)" : ")") << "\n";
    return true;

}

int main() {

    std::cout << "ScriptReader Tests:\n";

    const fs::path test_dir = "../tests/unit/script_reader";
    if (!fs::exists(test_dir)) {
        std::cerr << "Test directory not found: " << test_dir << "\n";
        return 1;
    }

    std::size_t pass_count {0}, fail_count {0};
    for (const auto& entry : fs::directory_iterator(test_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".sql") {
            if (round_trip_test(entry.path())) pass_count++;
            else fail_count++;
        }
    }

    std::cout << "Finished with " << pass_count << '/' << pass_count + fail_count
        << " passes." << std::endl;

    return !fail_count ? 0 : 1;
}