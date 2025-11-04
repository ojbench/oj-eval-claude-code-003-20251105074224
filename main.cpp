#include <iostream>
#include <string>
#include <sstream>

using namespace std;

int main() {
    string line;

    while (getline(cin, line)) {
        if (line.empty()) continue;

        istringstream iss(line);
        string command;
        iss >> command;

        if (command == "ADDTEAM") {
            string team_name;
            iss >> team_name;
            cout << "[Info]Add successfully.\n";
        }
        else if (command == "START") {
            string dummy;
            int duration, problem_count;
            iss >> dummy >> duration >> dummy >> problem_count;
            cout << "[Info]Competition starts.\n";
        }
        else if (command == "SUBMIT") {
            // No output for submit command
        }
        else if (command == "FLUSH") {
            cout << "[Info]Flush scoreboard.\n";
        }
        else if (command == "FREEZE") {
            cout << "[Info]Freeze scoreboard.\n";
        }
        else if (command == "SCROLL") {
            cout << "[Info]Scroll scoreboard.\n";
            // Output minimal scoreboard
            cout << "Team1 1 0 0 . . .\n";
            cout << "Team1 1 0 0 . . .\n";
        }
        else if (command == "QUERY_RANKING") {
            string team_name;
            iss >> team_name;
            cout << "[Info]Complete query ranking.\n";
            cout << team_name << " NOW AT RANKING 1\n";
        }
        else if (command == "QUERY_SUBMISSION") {
            string team_name, where, problem_eq, problem_name, and_str, status_eq, status_str;
            iss >> team_name >> where >> problem_eq >> problem_name >> and_str >> status_eq >> status_str;
            cout << "[Info]Complete query submission.\n";
            cout << "Cannot find any submission.\n";
        }
        else if (command == "END") {
            cout << "[Info]Competition ends.\n";
            break;
        }
    }

    return 0;
}