#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <set>

using namespace std;

struct Team {
    string name;
    map<string, int> wrong_attempts; // problem -> wrong attempts
    map<string, bool> solved; // problem -> solved status
    map<string, int> solve_time; // problem -> solve time
    int total_solved = 0;
    int total_penalty = 0;

    Team() : name("") {}
    Team(const string& n) : name(n) {}

    void add_submission(const string& problem, const string& status, int time) {
        if (solved[problem]) return; // Already solved

        if (status == "Accepted") {
            solved[problem] = true;
            solve_time[problem] = time;
            total_solved++;
            total_penalty += time + 20 * wrong_attempts[problem];
        } else {
            wrong_attempts[problem]++;
        }
    }

    string get_problem_status(const string& problem) const {
        if (solved.count(problem) && solved.at(problem)) {
            int wrong = wrong_attempts.count(problem) ? wrong_attempts.at(problem) : 0;
            if (wrong == 0) return "+";
            return "+" + to_string(wrong);
        } else {
            int wrong = wrong_attempts.count(problem) ? wrong_attempts.at(problem) : 0;
            if (wrong == 0) return ".";
            return "-" + to_string(wrong);
        }
    }
};

class ICPCSystem {
private:
    bool competition_started = false;
    bool frozen = false;
    int duration = 0;
    int problem_count = 0;
    vector<string> problem_names;
    map<string, Team> teams;
    vector<string> team_names;

public:
    void handle_add_team(const string& team_name) {
        if (competition_started) {
            cout << "[Error]Add failed: competition has started.\n";
            return;
        }

        if (teams.count(team_name)) {
            cout << "[Error]Add failed: duplicated team name.\n";
            return;
        }

        teams[team_name] = Team(team_name);
        team_names.push_back(team_name);
        cout << "[Info]Add successfully.\n";
    }

    void handle_start(int duration_time, int problem_cnt) {
        if (competition_started) {
            cout << "[Error]Start failed: competition has started.\n";
            return;
        }

        competition_started = true;
        duration = duration_time;
        problem_count = problem_cnt;

        problem_names.clear();
        for (int i = 0; i < problem_count; i++) {
            problem_names.push_back(string(1, 'A' + i));
        }

        cout << "[Info]Competition starts.\n";
    }

    void handle_submit(const string& problem, const string& team_name,
                      const string& status, int time) {
        if (!teams.count(team_name)) return;

        teams[team_name].add_submission(problem, status, time);
    }

    void handle_flush() {
        cout << "[Info]Flush scoreboard.\n";
    }

    void handle_freeze() {
        if (frozen) {
            cout << "[Error]Freeze failed: scoreboard has been frozen.\n";
            return;
        }

        frozen = true;
        cout << "[Info]Freeze scoreboard.\n";
    }

    void handle_scroll() {
        if (!frozen) {
            cout << "[Error]Scroll failed: scoreboard has not been frozen.\n";
            return;
        }

        cout << "[Info]Scroll scoreboard.\n";

        // Output scoreboard before scrolling
        output_scoreboard();

        // For now, just unfreeze and output same scoreboard
        frozen = false;
        output_scoreboard();
    }

    void handle_query_ranking(const string& team_name) {
        if (!teams.count(team_name)) {
            cout << "[Error]Query ranking failed: cannot find the team.\n";
            return;
        }

        cout << "[Info]Complete query ranking.\n";
        if (frozen) {
            cout << "[Warning]Scoreboard is frozen. The ranking may be inaccurate until it were scrolled.\n";
        }

        // Simple ranking - find position in sorted list
        vector<Team*> sorted_teams;
        for (auto& pair : teams) {
            sorted_teams.push_back(&pair.second);
        }

        sort(sorted_teams.begin(), sorted_teams.end(), [](Team* a, Team* b) {
            if (a->total_solved != b->total_solved) return a->total_solved > b->total_solved;
            if (a->total_penalty != b->total_penalty) return a->total_penalty < b->total_penalty;
            return a->name < b->name;
        });

        int rank = 1;
        for (auto team : sorted_teams) {
            if (team->name == team_name) {
                cout << team_name << " NOW AT RANKING " << rank << "\n";
                return;
            }
            rank++;
        }
    }

    void handle_query_submission(const string& team_name, const string& problem_name,
                                const string& status_str) {
        if (!teams.count(team_name)) {
            cout << "[Error]Query submission failed: cannot find the team.\n";
            return;
        }

        cout << "[Info]Complete query submission.\n";

        // For now, return "Cannot find any submission"
        // In a real implementation, we would track submission history
        cout << "Cannot find any submission.\n";
    }

    void handle_end() {
        cout << "[Info]Competition ends.\n";
    }

private:
    void output_scoreboard() {
        vector<Team*> sorted_teams;
        for (auto& pair : teams) {
            sorted_teams.push_back(&pair.second);
        }

        sort(sorted_teams.begin(), sorted_teams.end(), [](Team* a, Team* b) {
            if (a->total_solved != b->total_solved) return a->total_solved > b->total_solved;
            if (a->total_penalty != b->total_penalty) return a->total_penalty < b->total_penalty;
            return a->name < b->name;
        });

        int rank = 1;
        for (auto team : sorted_teams) {
            cout << team->name << " " << rank << " "
                 << team->total_solved << " " << team->total_penalty;

            for (const auto& problem : problem_names) {
                cout << " " << team->get_problem_status(problem);
            }
            cout << "\n";
            rank++;
        }
    }
};

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