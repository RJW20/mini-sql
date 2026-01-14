#include <minisql/script_reader.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "unit_test_paths.hpp"

namespace fs = std::filesystem;

bool round_trip_test(const fs::path& filename) {

    const fs::path FIRST_PASS_OUTPUT = "first_path_output.sql";

    // Read original file
    minisql::ScriptReader script_1{filename};
    if (!script_1.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }
    std::vector<std::string> statements_1;
    while (auto statement = script_1.next())
        statements_1.push_back(*statement);

    // Second pass on first output
    {
        std::ofstream output{FIRST_PASS_OUTPUT};
        for (const std::string& s : statements_1) output << s << '\n';
    }
    std::vector<std::string> statements_2;
    {
        minisql::ScriptReader script_2{FIRST_PASS_OUTPUT};
        while (auto statement = script_2.next())
            statements_2.push_back(*statement);
    }
    fs::remove(FIRST_PASS_OUTPUT);

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

    const fs::path test_dir = fs::path{UNIT_TEST_ROOT} / "script_reader";
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