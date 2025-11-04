#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

// Simple implementation focusing on basic functionality
class ICPCSystem {
private:
    bool competition_started = false;
    bool is_frozen = false;
    int duration_time = 0;
    int problem_count = 0;
    vector<string> problem_names;

    map<string, vector<string>> team_submissions; // team -> list of submissions
    vector<string> teams;

public:
    ICPCSystem() = default;

    // Command handlers
    void handle_add_team(const string& team_name) {
        if (competition_started) {
            cout << "[Error]Add failed: competition has started.\n";
            return;
        }

        if (find(teams.begin(), teams.end(), team_name) != teams.end()) {
            cout << "[Error]Add failed: duplicated team name.\n";
            return;
        }

        teams.push_back(team_name);
        team_submissions[team_name] = vector<string>();
        cout << "[Info]Add successfully.\n";
    }

    void handle_start(int duration, int problem_cnt) {
        if (competition_started) {
            cout << "[Error]Start failed: competition has started.\n";
            return;
        }

        competition_started = true;
        duration_time = duration;
        problem_count = problem_cnt;

        // Generate problem names (A, B, C, ...)
        for (int i = 0; i < problem_count; i++) {
            problem_names.push_back(string(1, 'A' + i));
        }

        cout << "[Info]Competition starts.\n";
    }

    void handle_submit(const string& problem_name, const string& team_name,
                      const string& status_str, int time) {
        // Simple submission recording
        string submission = team_name + " " + problem_name + " " + status_str + " " + to_string(time);
        team_submissions[team_name].push_back(submission);
        // No output for submit command
    }

    void handle_flush() {
        cout << "[Info]Flush scoreboard.\n";
    }

    void handle_freeze() {
        if (is_frozen) {
            cout << "[Error]Freeze failed: scoreboard has been frozen.\n";
            return;
        }

        is_frozen = true;
        cout << "[Info]Freeze scoreboard.\n";
    }

    void handle_scroll() {
        if (!is_frozen) {
            cout << "[Error]Scroll failed: scoreboard has not been frozen.\n";
            return;
        }

        cout << "[Info]Scroll scoreboard.\n";
        // Simple output for now
        for (const auto& team : teams) {
            cout << team << " 1 0 0";
            for (int i = 0; i < problem_count; i++) {
                cout << " .";
            }
            cout << "\n";
        }

        // Output again after "scrolling"
        for (const auto& team : teams) {
            cout << team << " 1 0 0";
            for (int i = 0; i < problem_count; i++) {
                cout << " .";
            }
            cout << "\n";
        }
    }

    void handle_query_ranking(const string& team_name) {
        if (find(teams.begin(), teams.end(), team_name) == teams.end()) {
            cout << "[Error]Query ranking failed: cannot find the team.\n";
            return;
        }

        cout << "[Info]Complete query ranking.\n";
        if (is_frozen) {
            cout << "[Warning]Scoreboard is frozen. The ranking may be inaccurate until it were scrolled.\n";
        }

        // Simple ranking - just return 1 for now
        cout << team_name << " NOW AT RANKING 1\n";
    }

    void handle_query_submission(const string& team_name, const string& problem_name,
                                const string& status_str) {
        if (find(teams.begin(), teams.end(), team_name) == teams.end()) {
            cout << "[Error]Query submission failed: cannot find the team.\n";
            return;
        }

        cout << "[Info]Complete query submission.\n";

        const auto& submissions = team_submissions[team_name];

        // Find the last matching submission
        for (auto it = submissions.rbegin(); it != submissions.rend(); ++it) {
            istringstream iss(*it);
            string t_name, p_name, status, time_str;
            iss >> t_name >> p_name >> status >> time_str;

            bool problem_match = (problem_name == "ALL" || p_name == problem_name);
            bool status_match = (status_str == "ALL" || status == status_str);

            if (problem_match && status_match) {
                cout << *it << "\n";
                return;
            }
        }

        cout << "Cannot find any submission.\n";
    }

    void handle_end() {
        cout << "[Info]Competition ends.\n";
    }
};

// Main function to process commands
int main() {
    ICPCSystem system;
    string line;

    while (getline(cin, line)) {
        if (line.empty()) continue;

        istringstream iss(line);
        string command;
        iss >> command;

        if (command == "ADDTEAM") {
            string team_name;
            iss >> team_name;
            system.handle_add_team(team_name);
        }
        else if (command == "START") {
            string dummy;
            int duration, problem_count;
            iss >> dummy >> duration >> dummy >> problem_count;
            system.handle_start(duration, problem_count);
        }
        else if (command == "SUBMIT") {
            string problem_name, by, team_name, with, status_str, at;
            int time;
            iss >> problem_name >> by >> team_name >> with >> status_str >> at >> time;
            system.handle_submit(problem_name, team_name, status_str, time);
        }
        else if (command == "FLUSH") {
            system.handle_flush();
        }
        else if (command == "FREEZE") {
            system.handle_freeze();
        }
        else if (command == "SCROLL") {
            system.handle_scroll();
        }
        else if (command == "QUERY_RANKING") {
            string team_name;
            iss >> team_name;
            system.handle_query_ranking(team_name);
        }
        else if (command == "QUERY_SUBMISSION") {
            string team_name, where, problem_eq, problem_name, and_str, status_eq, status_str;
            iss >> team_name >> where >> problem_eq >> problem_name >> and_str >> status_eq >> status_str;
            // Remove "PROBLEM=" and "STATUS=" prefixes
            problem_name = problem_name.substr(8);
            status_str = status_str.substr(7);
            system.handle_query_submission(team_name, problem_name, status_str);
        }
        else if (command == "END") {
            system.handle_end();
            break;
        }
    }

    return 0;
}