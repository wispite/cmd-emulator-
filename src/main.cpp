#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

vector<string> parser(const string& input) {
    vector<string> tokens;
    string current;
    bool quoted = false;
    bool quote = false;

    for (char c : input) {
        if (c == '"') {
            quoted = !quoted;// если первая кавычка то true и будет true пока не встретит вторую кавычку
            quote = true;
        }

        else if (c == ' ' && !quoted) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            if (c == ' ' && quoted) current.push_back(' ');
        } else current += c;
    }
    if (!current.empty()) tokens.push_back(current);

    if (quoted) throw runtime_error("unclosed quotes are not allowed");
    if (tokens.empty() && quote) throw runtime_error("empty quotes are not allowed");
    if (tokens.empty()) throw runtime_error("just backspaces... are you serious?");

    return tokens;
}

void prntCmd(const vector<string>& tokens) {
    cout << "command: " << tokens[0] << endl;
    cout << "arguments:";

    if (tokens.size() == 1) cout << " none";
    else {
        for (size_t i = 1; i < tokens.size(); i++) {
            cout << " [" << tokens[i] << "]";
        }
    }
    cout << endl;
}


int main() {
    const string VFSnaming = "myVFS";

    cout << "...cmd emulator started" << endl;
    cout << "'exit' to quit" << endl;

    while (true) {
        string input;
        cout << VFSnaming << ":~$ ";
        getline(cin, input);

        if (input.empty()) continue;

        try {
            vector<string> tokens = parser(input);
            string cmd = tokens[0];

            if (cmd == "exit") {
                cout << "bye bye! ;)" << endl;
                break;
            }

            if (cmd == "ls") prntCmd(tokens);
            else if (cmd == "cd") prntCmd(tokens);
            else cout << "err: unknown command '" << cmd << "'" << endl;

        }
        catch (const exception& e) {
            cout << "err: " << e.what() << endl;
        }
    }
    return 0;
}