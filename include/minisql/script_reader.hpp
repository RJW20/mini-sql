#ifndef MINISQL_SCRIPT_READER_HPP
#define MINISQL_SCRIPT_READER_HPP

#include <cstddef>
#include <istream>
#include <optional>
#include <string>
#include <algorithm>

namespace minisql {

/* Script Reader
 * Yields all ; separated statements from the given script.
 * Ignores all characters between any # and the next \n. */
class ScriptReader {
public:
    explicit ScriptReader(std::istream& script) : script_{script} {}

    std::optional<std::string> next() {
        if (std::optional<std::string> statement = extract_statement())
            return statement;
        std::string line;
        while (std::getline(script_, line)) {
            strip_comments(line);
            normalise_whitespace(line);
            buffer_ += ' ';
            buffer_ += line;
            line.clear();
            if (std::optional<std::string> statement = extract_statement())
                return statement;
        }
        return std::nullopt;
    }

private:
    std::istream& script_;
    std::string buffer_;
    bool inside_quote_ {false};
    std::size_t buffer_pos_ {0};

    std::optional<std::string> extract_statement() {
        for (std::size_t i = buffer_pos_; i < buffer_.size(); i++) {
            char c = buffer_[i];
            if (c == '"') inside_quote_ = !inside_quote_;
            if (c == STATEMENT_SEPARATOR && !inside_quote_) {
                std::string statement = buffer_.substr(0, i + 1);
                buffer_.erase(0, i + 1);
                buffer_pos_ = 0;
                trim(statement);
                if (statement[0] != STATEMENT_SEPARATOR) return statement;
                return extract_statement();
            }
        }
        buffer_pos_ = buffer_.size();
        return std::nullopt;
    }

    static void strip_comments(std::string& line) {
        std::size_t pos = line.find(COMMENT_BEGIN);
        if (pos != std::string::npos) line.erase(pos);
    }

    static void normalise_whitespace(std::string& line) {
        for (char& c : line) if (c == '\n' || c == '\t' || c == '\r') c = ' ';
    }

    static void trim(std::string& s) {
        auto not_space = [](unsigned char ch){ return !std::isspace(ch); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    }

    static constexpr char COMMENT_BEGIN = '#';
    static constexpr char STATEMENT_SEPARATOR = ';';
};

} // namespace minisql

#endif // MINISQL_SCRIPT_READER_HPP