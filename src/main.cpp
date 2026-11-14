#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {
constexpr const char* reset = "\033[0m";
constexpr const char* blue = "\033[1;34m";
constexpr const char* green = "\033[32m";
constexpr const char* yellow = "\033[33m";
constexpr const char* gray = "\033[90m";

const std::set<std::string> keywords{
    "fn", "let", "mut", "if", "else", "while", "for", "in", "return",
    "true", "false", "struct", "enum", "import", "as", "break", "continue",
    "and", "or", "not"
};

bool isIdentifierStart(char ch) {
    const auto c = static_cast<unsigned char>(ch);
    return std::isalpha(c) != 0 || ch == '_';
}

bool isIdentifierPart(char ch) {
    const auto c = static_cast<unsigned char>(ch);
    return std::isalnum(c) != 0 || ch == '_';
}

std::string colorize(const std::string& line, bool colors) {
    if (!colors) return line;

    std::string result;
    for (std::size_t i = 0; i < line.size();) {
        if (line[i] == '/' && i + 1 < line.size() && line[i + 1] == '/') {
            result += gray;
            result.append(line, i, std::string::npos);
            result += reset;
            break;
        }

        if (line[i] == '"' || line[i] == '\'') {
            const char quote = line[i];
            result += green;
            result += line[i++];
            while (i < line.size()) {
                const char current = line[i++];
                result += current;
                if (current == '\\' && i < line.size()) {
                    result += line[i++];
                } else if (current == quote) {
                    break;
                }
            }
            result += reset;
            continue;
        }

        if (isIdentifierStart(line[i])) {
            const std::size_t start = i++;
            while (i < line.size() && isIdentifierPart(line[i])) ++i;
            const std::string token = line.substr(start, i - start);
            if (keywords.count(token) != 0) result += blue;
            result += token;
            if (keywords.count(token) != 0) result += reset;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(line[i])) != 0) {
            result += yellow;
            while (i < line.size() &&
                   (std::isdigit(static_cast<unsigned char>(line[i])) != 0 || line[i] == '.')) {
                result += line[i++];
            }
            result += reset;
            continue;
        }

        result += line[i++];
    }
    return result;
}

bool loadFile(const fs::path& path, std::vector<std::string>& lines, std::string& error) {
    std::ifstream input(path);
    if (!input) {
        error = "Could not open '" + path.string() + "'.";
        return false;
    }
    std::vector<std::string> loaded;
    std::string line;
    while (std::getline(input, line)) loaded.push_back(line);
    if (input.bad()) {
        error = "An error occurred while reading '" + path.string() + "'.";
        return false;
    }
    lines = std::move(loaded);
    return true;
}

bool saveFile(const fs::path& path, const std::vector<std::string>& lines, std::string& error) {
    std::ofstream output(path, std::ios::trunc);
    if (!output) {
        error = "Could not write '" + path.string() + "'.";
        return false;
    }
    for (const auto& line : lines) output << line << '\n';
    if (!output) {
        error = "An error occurred while writing '" + path.string() + "'.";
        return false;
    }
    return true;
}

bool parseLineNumber(const std::string& text, std::size_t& number) {
    try {
        std::size_t consumed = 0;
        const auto parsed = std::stoull(text, &consumed);
        if (consumed != text.size() || parsed == 0) return false;
        number = static_cast<std::size_t>(parsed);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void showHelp() {
    std::cout << "Asimov Editor commands:\n"
              << "  :list             show the source buffer\n"
              << "  :insert N         insert a line before line N, then type the line\n"
              << "  :replace N        replace line N, then type the new line\n"
              << "  :delete N         delete line N\n"
              << "  :open PATH        load a source file\n"
              << "  :save [PATH]      save the buffer\n"
              << "  :help             show this help\n"
              << "  :quit             exit the editor\n"
              << "Any other input is appended as a source line.\n";
}
} // namespace

int main(int argc, char* argv[]) {
    std::vector<std::string> lines;
    fs::path currentPath;
    bool dirty = false;
    bool colors = std::getenv("NO_COLOR") == nullptr;

    if (argc > 2) {
        std::cerr << "Usage: asimov-editor [file.asimov] [--no-color]\n";
        return 2;
    }
    if (argc == 2) {
        const std::string argument = argv[1];
        if (argument == "--no-color") {
            colors = false;
        } else {
            currentPath = argument;
            if (fs::exists(currentPath)) {
                std::string error;
                if (!loadFile(currentPath, lines, error)) {
                    std::cerr << error << '\n';
                    return 1;
                }
            }
        }
    }

    std::cout << "Asimov Editor 0.1 — :help for commands\n";
    if (!currentPath.empty()) std::cout << "File: " << currentPath.string() << '\n';

    std::string input;
    while (true) {
        std::cout << "asimov> " << std::flush;
        if (!std::getline(std::cin, input)) break;

        if (input == ":help") {
            showHelp();
        } else if (input == ":list") {
            if (lines.empty()) std::cout << "(empty buffer)\n";
            for (std::size_t i = 0; i < lines.size(); ++i) {
                std::cout << (i + 1) << " | " << colorize(lines[i], colors) << '\n';
            }
        } else if (input.rfind(":insert ", 0) == 0 || input.rfind(":replace ", 0) == 0) {
            const bool replace = input.rfind(":replace ", 0) == 0;
            const std::size_t prefixLength = replace ? 9 : 8;
            std::size_t number = 0;
            if (!parseLineNumber(input.substr(prefixLength), number) ||
                number > lines.size() + (replace ? 0U : 1U)) {
                std::cout << "Invalid line number. Valid range: 1-"
                          << (lines.size() + (replace ? 0 : 1)) << ".\n";
                continue;
            }
            std::cout << "line> " << std::flush;
            if (!std::getline(std::cin, input)) break;
            if (replace) lines[number - 1] = input;
            else lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(number - 1), input);
            dirty = true;
        } else if (input.rfind(":delete ", 0) == 0) {
            std::size_t number = 0;
            if (!parseLineNumber(input.substr(8), number) || number > lines.size()) {
                std::cout << "Invalid line number. Valid range: 1-" << lines.size() << ".\n";
                continue;
            }
            lines.erase(lines.begin() + static_cast<std::ptrdiff_t>(number - 1));
            dirty = true;
        } else if (input.rfind(":open ", 0) == 0) {
            const fs::path path = input.substr(6);
            std::vector<std::string> loaded;
            std::string error;
            if (dirty) {
                std::cout << "Discard unsaved changes? [y/N] " << std::flush;
                std::string answer;
                if (!std::getline(std::cin, answer) || (answer != "y" && answer != "Y")) continue;
            }
            if (loadFile(path, loaded, error)) {
                lines = std::move(loaded);
                currentPath = path;
                dirty = false;
                std::cout << "Opened " << currentPath.string() << " (" << lines.size() << " lines).\n";
            } else {
                std::cout << error << '\n';
            }
        } else if (input == ":save" || input.rfind(":save ", 0) == 0) {
            if (input.size() > 5) currentPath = input.substr(6);
            if (currentPath.empty()) {
                std::cout << "No file selected. Use :save PATH.\n";
                continue;
            }
            std::string error;
            if (saveFile(currentPath, lines, error)) {
                dirty = false;
                std::cout << "Saved " << currentPath.string() << ".\n";
            } else {
                std::cout << error << '\n';
            }
        } else if (input == ":quit") {
            if (dirty) {
                std::cout << "Discard unsaved changes? [y/N] " << std::flush;
                std::string answer;
                if (!std::getline(std::cin, answer) || (answer != "y" && answer != "Y")) continue;
            }
            break;
        } else if (!input.empty() && input.front() == ':') {
            std::cout << "Unknown command. Type :help for available commands.\n";
        } else {
            lines.push_back(input);
            dirty = true;
        }
    }

    return 0;
}
