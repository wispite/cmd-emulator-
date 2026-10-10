#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>

using namespace std;

struct Config {
    string vfsPath = "./myVFS";
    string prompt = "myVFS:~$ ";
    string scriptPath;
};

vector<string> parser(const string& input) {
    vector<string> tokens;
    string current;

    bool quoted = false;
    bool tokenStarted = false;
    size_t quoteContentStart = 0;

    for (char c : input) {
        //открытие или закрытие кавычек
        if (c == '"') {
            if (!quoted) {
                quoted = true;
                tokenStarted = true;
                quoteContentStart = current.size();
            } else {
                //проверяем, не состоят ли кавычки только из пробелов
                bool onlySpaces = all_of(
                    current.begin() + quoteContentStart,
                    current.end(),
                    [](unsigned char ch) {
                        return isspace(ch);
                    }
                );

                if (onlySpaces) throw runtime_error("empty quotes");
                quoted = false;
            }
        }

        //  пробел вне кавычек завершает аргумент
        else if (isspace(static_cast<unsigned char>(c)) && !quoted) {
            if (tokenStarted) {
                tokens.push_back(current);
                current.clear();
                tokenStarted = false;
            }
        }
        //обычный символ или пробел внутри кавычек
        else {
            current += c;
            tokenStarted = true;
        }
    }

    if (quoted) throw runtime_error("unclosed quotes");
    if (tokenStarted) tokens.push_back(current);
    if (tokens.empty()) throw runtime_error("empty command");

    return tokens;
}

void printCommand(const vector<string>& tokens) {
    cout << "command: " << tokens[0] << '\n';
    cout << "arguments:";
    if (tokens.size() == 1) {
        cout << " none";
    } else {
        for (size_t i = 1; i < tokens.size(); ++i) {
            cout << " [" << tokens[i] << "]";
        }
    }
    cout << '\n';
}

void confDump(const Config& config) {
    cout << "vfs=" << config.vfsPath << '\n';
    cout << "prompt=" << config.prompt << '\n';
    cout << "script=";
    if (config.scriptPath.empty()) cout << "(not set)";
    else cout << config.scriptPath;
    cout << '\n';
}

// Returns false when the command is "exit"; errors are reported to the caller.
bool executeCommand(const string& input, const Config& config) {
    vector<string> tokens = parser(input);
    if (tokens.empty()) return true;

    const string& cmd = tokens[0];
    if (cmd == "exit") {
        if (tokens.size() != 1) throw runtime_error("exit: too many arguments");
        cout << "bye bye! ;)" << '\n';
        return false;
    }
    if (cmd == "ls" || cmd == "cd") {
        printCommand(tokens);
    } else if (cmd == "conf-dump") {
        if (tokens.size() != 1) throw runtime_error("conf-dump: no arguments expected");
        confDump(config);
    } else {
        throw runtime_error("unknown command '" + cmd + "'");
    }
    return true;
}

bool runScript(const Config& config) {
    ifstream script(config.scriptPath);
    if (!script.is_open()) {
        cerr << "err: cannot open startup script '" << config.scriptPath << "'\n";
        return false;
    }

    string line;
    size_t lineNumber = 0;
    while (getline(script, line)) {
        ++lineNumber;
        if (line.find_first_not_of(" \t\r") == string::npos) continue;

        // Show the scripted input as if the user typed it into the REPL.
        cout << config.prompt << line << '\n';
        try {
            if (!executeCommand(line, config)) return true;
        } catch (const exception& e) {
            cerr << "err (script line " << lineNumber << "): " << e.what() << '\n';
            // Requirement for variant 2: skip the bad line and continue.
        }
    }
    return true;
}

bool parseArguments(int argc, char* argv[], Config& config) {
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            cout << "Usage: " << argv[0]
                 << " [--vfs PATH] [--prompt TEXT] [--script PATH]\n";
            return false;
        }

        if (arg == "--vfs" || arg == "--prompt" || arg == "--script") {
            if (i + 1 >= argc) {
                throw runtime_error("missing value for " + arg);
            }
            string value = argv[++i];
            if (arg == "--vfs") config.vfsPath = value;
            else if (arg == "--prompt") config.prompt = value;
            else config.scriptPath = value;
        } else {
            throw runtime_error("unknown argument '" + arg + "'");
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    Config config;

    try {
        if (!parseArguments(argc, argv, config)) return 0;
    } catch (const exception& e) {
        cerr << "err: " << e.what() << '\n';
        cerr << "Use --help to see available parameters.\n";
        return 1;
    }

    cout << "...cmd emulator started\n";
    cout << "Configuration (debug):\n";
    confDump(config);
    cout << "Type 'exit' to quit.\n";

    if (!config.scriptPath.empty()) {
        if (!runScript(config)) return 1;
    }

    while (true) {
        string input;
        cout << config.prompt;
        if (!getline(cin, input)) break;
        if (input.find_first_not_of(" \t\r") == string::npos) continue;

        try {
            if (!executeCommand(input, config)) break;
        } catch (const exception& e) {
            cout << "err: " << e.what() << '\n';
        }
    }

    return 0;
}
