#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <minisql/connection.hpp>
#include <minisql/script_reader.hpp>

#include "directory_iterable.hpp"
#include "executor.hpp"
#include "integration_test_paths.hpp"

namespace fs = std::filesystem;

// ANSI colors
const std::string COLOR_RESET = "\033[0m";
const std::string COLOR_RED   = "\033[31m";
const std::string COLOR_GREEN = "\033[32m";
const std::string COLOR_YELLOW= "\033[33m";
const std::string COLOR_BOLD  = "\033[1m";

struct TestResult {
    std::string name;
    bool passed;
    std::size_t line;
    std::string expected;
    std::string got;
};

TestResult run_test(const fs::path& filename) {

    std::string name = filename.stem().string();
    TestResult result{name, false, 0};

    minisql::ScriptReader script{filename};
    std::ifstream output{fs::path{INTEGRATION_TEST_OUTPUT} / (name + ".out")};
    if (!script.is_open() || !output.is_open()) {
        std::cerr << "Failed to open files for test: " << name << "\n";
        return result;
    }

    const fs::path db_path = name + ".db";
    {
        minisql::Connection connection{db_path};
        Executor executor{script, connection};
        while (true) {
            result.line++;
            if (!std::getline(output, result.expected)) result.expected = "";
            auto got = executor.next();
            if (!got) result.got = "";
            else result.got = *got;
            if (result.expected.empty() && result.got.empty()) {
                result.passed = true;
                break;
            }
            if (result.expected != result.got) break;
        }
    }

    fs::remove(db_path);
    return result;
}

/* Run scripts in scripts/ and compare the output with the corresponding file
 * in output/.
 * argv allows selection of specific prefixes of scripts to run, with order
 * being maintained. For example:
 * - 001 runs just script 001_{name}.sql
 * - 001, 002 runs scripts 001_{name}.sql and 002_{name}.sql
 * - 0 runs all scripts 0xx_{name}.sql */
int main(int argc, char* argv[]) {

    const fs::path script_dir = INTEGRATION_TEST_SCRIPTS;
    DirectoryIterable scripts = (argc > 1)
        ? DirectoryIterable(
            script_dir,
            std::vector<std::string>(argv + 1, argv + argc)
        ) : DirectoryIterable(script_dir);

    std::size_t pass_count {0}, fail_count {0};
    for (const fs::path& script : scripts) {
        if (script.extension() != ".sql") continue;
        TestResult r = run_test(script);
        if (r.passed) {
            pass_count++;
            std::cout << COLOR_GREEN << "[PASS]" << COLOR_RESET << " "
                << r.name << "\n";
        }
        else {
            fail_count++;
            std::cout << COLOR_RED << "[FAIL]" << COLOR_RESET << " " << r.name
                << " on output line " << r.line << "\n";
            std::cout << "  " << COLOR_YELLOW << "Expected: " << COLOR_RESET
                << r.expected << "\n";
            std::cout << "  " << COLOR_YELLOW << "Got:      " << COLOR_RESET
                << r.got << "\n";
        }
    }

    std::cout << "\n" << COLOR_BOLD << (fail_count ? COLOR_RED : COLOR_GREEN);
    std::cout << "Summary: " << pass_count << "/" << pass_count + fail_count
        << " tests passed." << COLOR_RESET << "\n";

    return !fail_count ? 0 : 1;
}