
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <cctype>
#include <dirent.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

using namespace std;

struct Config {
    string vfsPath = "./myVFS";
    string prompt = "myVFS:~$ ";
    string scriptPath;
};


struct Node {
    string name;
    bool isDirectory;
    string content;

    Node* parent;
    vector<unique_ptr<Node>> children;

    Node(const string& name, bool isDirectory, Node* parent = nullptr)
        : name(name), isDirectory(isDirectory), parent(parent) {}
};

struct VFS {
    unique_ptr<Node> root;
    Node* current = nullptr;
};


string getBaseName(string path) {
    while (path.size() > 1 && path.back() == '/') {
        path.pop_back();
    }

    size_t pos = path.find_last_of('/');

    if (pos == string::npos) return path;
    if (pos == path.size() - 1) return "/";

    return path.substr(pos + 1);
}

string joinPath(const string& parent, const string& child) {
    if (parent.empty() || parent == "/") {
        return "/" + child;
    }

    return parent + "/" + child;
}

string readFile(const string& path) {
    ifstream file(path, ios::binary);

    if (!file.is_open()) {
        throw runtime_error("cannot open file: " + path);
    }

    ostringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}


unique_ptr<Node> loadNode(
    const string& path,
    const string& name,
    Node* parent
) {
    struct stat info{};

    if (lstat(path.c_str(), &info) != 0) {
        throw runtime_error(
            "cannot access: " + path + ": " + strerror(errno)
        );
    }

    bool isDirectory = S_ISDIR(info.st_mode);

    auto node = make_unique<Node>(name, isDirectory, parent);

    if (!isDirectory) {
        node->content = readFile(path);
        return node;
    }

    DIR* dir = opendir(path.c_str());

    if (dir == nullptr) {
        throw runtime_error(
            "cannot open directory: " + path + ": " + strerror(errno)
        );
    }

    errno = 0;

    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        string childName = entry->d_name;

        if (childName == "." || childName == "..") {
            continue;
        }

        string childPath = joinPath(path, childName);

        node->children.push_back(
            loadNode(childPath, childName, node.get())
        );

        errno = 0;
    }

    int readError = errno;
    closedir(dir);

    if (readError != 0) {
        throw runtime_error(
            "error reading directory: " + path
        );
    }

    return node;
}

unique_ptr<Node> loadDirectory(const string& path) {
    return loadNode(path, getBaseName(path), nullptr);
}


void printTree(const Node* node, int depth = 0) {
    if (node == nullptr) return;

    for (int i = 0; i < depth; ++i) {
        cout << "  ";
    }

    if (node->isDirectory) {
        cout << "[DIR] ";
    } else {
        cout << "[FILE] ";
    }

    cout << node->name << '\n';

    for (const auto& child : node->children) {
        printTree(child.get(), depth + 1);
    }
}


vector<string> parser(const string& input) {
    vector<string> tokens;
    string current;

    bool quoted = false;
    bool tokenStarted = false;
    size_t quoteContentStart = 0;

    for (char c : input) {
        if (c == '"') {
            if (!quoted) {
                quoted = true;
                tokenStarted = true;
                quoteContentStart = current.size();
            } else {
                bool onlySpaces = all_of(
                    current.begin() + quoteContentStart,
                    current.end(),
                    [](unsigned char ch) {
                        return isspace(ch);
                    }
                );

                if (onlySpaces) {
                    throw runtime_error("empty quotes");
                }

                quoted = false;
            }
        }
        else if (isspace(static_cast<unsigned char>(c)) && !quoted) {
            if (tokenStarted) {
                tokens.push_back(current);
                current.clear();
                tokenStarted = false;
            }
        }
        else {
            current += c;
            tokenStarted = true;
        }
    }

    if (quoted) {
        throw runtime_error("unclosed quotes");
    }

    if (tokenStarted) {
        tokens.push_back(current);
    }

    if (tokens.empty()) {
        throw runtime_error("empty command");
    }

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

    if (config.scriptPath.empty()) {
        cout << "(not set)";
    } else {
        cout << config.scriptPath;
    }

    cout << '\n';
}

bool executeCommand(const string& input, const Config& config) {
    vector<string> tokens = parser(input);

    const string& cmd = tokens[0];

    if (cmd == "exit") {
        if (tokens.size() != 1) {
            throw runtime_error("exit: too many arguments");
        }

        cout << "bye bye! ;)\n";
        return false;
    }

    if (cmd == "conf-dump") {
        if (tokens.size() != 1) {
            throw runtime_error("conf-dump: no arguments expected");
        }

        confDump(config);
    }
    else if (cmd == "ls" || cmd == "cd") {
        // Diagnostic implementation for stage 3.
        // Full VFS commands will be implemented in stage 4.
        printCommand(tokens);
    }
    else {
        throw runtime_error("unknown command '" + cmd + "'");
    }

    return true;
}

bool runScript(const Config& config, bool& shouldExit) {
    ifstream script(config.scriptPath);

    if (!script.is_open()) {
        cerr << "err: cannot open startup script '"
             << config.scriptPath << "'\n";
        return false;
    }

    string line;
    size_t lineNumber = 0;

    while (getline(script, line)) {
        ++lineNumber;

        if (line.find_first_not_of(" \t\r") == string::npos) {
            continue;
        }

        cout << config.prompt << line << '\n';

        try {
            if (!executeCommand(line, config)) {
                shouldExit = true;
                return true;
            }
        }
        catch (const exception& e) {
            cerr << "err (script line "
                 << lineNumber << "): " << e.what() << '\n';
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

            if (arg == "--vfs") {
                config.vfsPath = value;
            }
            else if (arg == "--prompt") {
                config.prompt = value;
            }
            else {
                config.scriptPath = value;
            }
        }
        else {
            throw runtime_error("unknown argument '" + arg + "'");
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    Config config;

    try {
        if (!parseArguments(argc, argv, config)) {
            return 0;
        }

        cout << "...cmd emulator started\n";
        cout << "Configuration (debug):\n";

        confDump(config);

        cout << "Type 'exit' to quit.\n";

        // Create and load virtual file system.
        VFS vfs;

        vfs.root = loadDirectory(config.vfsPath);
        vfs.current = vfs.root.get();

        // Display loaded VFS.
        printTree(vfs.root.get());

        bool shouldExit = false;

        if (!config.scriptPath.empty()) {
            if (!runScript(config, shouldExit)) {
                return 1;
            }

            if (shouldExit) {
                return 0;
            }
        }

        // Interactive command loop.
        while (true) {
            string input;

            cout << config.prompt;

            if (!getline(cin, input)) {
                break;
            }

            if (input.find_first_not_of(" \t\r") == string::npos) {
                continue;
            }

            try {
                if (!executeCommand(input, config)) {
                    break;
                }
            }
            catch (const exception& e) {
                cout << "err: " << e.what() << '\n';
            }
        }
    }
    catch (const exception& e) {
        cerr << "err: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
