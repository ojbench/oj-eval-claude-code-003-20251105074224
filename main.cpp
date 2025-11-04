#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <memory>
#include <cassert>

using namespace std;

// Judge status enumeration
enum class JudgeStatus {
    Accepted,
    Wrong_Answer,
    Runtime_Error,
    Time_Limit_Exceed
};

// Convert string to JudgeStatus
JudgeStatus stringToStatus(const string& status) {
    if (status == "Accepted") return JudgeStatus::Accepted;
    if (status == "Wrong_Answer") return JudgeStatus::Wrong_Answer;
    if (status == "Runtime_Error") return JudgeStatus::Runtime_Error;
    if (status == "Time_Limit_Exceed") return JudgeStatus::Time_Limit_Exceed;
    return JudgeStatus::Wrong_Answer; // Default fallback
}

// Convert JudgeStatus to string
string statusToString(JudgeStatus status) {
    switch (status) {
        case JudgeStatus::Accepted: return "Accepted";
        case JudgeStatus::Wrong_Answer: return "Wrong_Answer";
        case JudgeStatus::Runtime_Error: return "Runtime_Error";
        case JudgeStatus::Time_Limit_Exceed: return "Time_Limit_Exceed";
        default: return "Unknown";
    }
}

// Team submission record
struct Submission {
    string problem_name;
    JudgeStatus status;
    int time;

    Submission(const string& p, JudgeStatus s, int t)
        : problem_name(p), status(s), time(t) {}
};

// Team problem status
struct ProblemStatus {
    bool solved = false;
    int first_solve_time = -1;
    int wrong_attempts = 0;
    int frozen_wrong_attempts = 0; // wrong attempts before freezing
    int frozen_submissions = 0;    // submissions during freezing

    bool is_frozen() const {
        return frozen_submissions > 0;
    }

    int total_wrong_attempts() const {
        return wrong_attempts + frozen_wrong_attempts;
    }

    int penalty_time() const {
        if (!solved) return 0;
        return 20 * wrong_attempts + first_solve_time;
    }
};

// Team class
class Team {
public:
    string name;
    map<string, ProblemStatus> problems;
    vector<Submission> submissions;

    // For ranking calculation (excluding frozen problems)
    int solved_count = 0;
    int total_penalty = 0;
    vector<int> solve_times;

    Team(const string& n) : name(n) {}

    // Update team status based on submission
    void add_submission(const string& problem_name, JudgeStatus status, int time, bool is_frozen) {
        submissions.emplace_back(problem_name, status, time);

        auto& problem = problems[problem_name];

        if (problem.solved) {
            // Already solved, no further updates needed
            return;
        }

        if (is_frozen) {
            problem.frozen_submissions++;
            if (status != JudgeStatus::Accepted) {
                problem.frozen_wrong_attempts++;
            }
        } else {
            if (status == JudgeStatus::Accepted) {
                problem.solved = true;
                problem.first_solve_time = time;
                // Update ranking stats
                solved_count++;
                total_penalty += problem.penalty_time();
                solve_times.push_back(time);
                sort(solve_times.begin(), solve_times.end(), greater<int>());
            } else {
                problem.wrong_attempts++;
            }
        }
    }

    // Get problem display string
    string get_problem_display(const string& problem_name) const {
        const auto it = problems.find(problem_name);
        if (it == problems.end()) {
            return ".";
        }

        const auto& problem = it->second;

        if (problem.is_frozen()) {
            if (problem.wrong_attempts == 0) {
                return "0/" + to_string(problem.frozen_submissions);
            } else {
                return "-" + to_string(problem.wrong_attempts) + "/" + to_string(problem.frozen_submissions);
            }
        } else if (problem.solved) {
            if (problem.wrong_attempts == 0) {
                return "+";
            } else {
                return "+" + to_string(problem.wrong_attempts);
            }
        } else {
            if (problem.wrong_attempts == 0) {
                return ".";
            } else {
                return "-" + to_string(problem.wrong_attempts);
            }
        }
    }
};

// ICPC Management System
class ICPCSystem {
private:
    bool competition_started = false;
    bool is_frozen = false;
    int duration_time = 0;
    int problem_count = 0;
    vector<string> problem_names;

    map<string, shared_ptr<Team>> teams;
    vector<shared_ptr<Team>> team_list; // For maintaining order

    // Scoreboard state
    vector<shared_ptr<Team>> scoreboard;

public:
    ICPCSystem() = default;

    // Command handlers
    void handle_add_team(const string& team_name) {
        if (competition_started) {
            cout << "[Error]Add failed: competition has started.\n";
            return;
        }

        if (teams.find(team_name) != teams.end()) {
            cout << "[Error]Add failed: duplicated team name.\n";
            return;
        }

        auto team = make_shared<Team>(team_name);
        teams[team_name] = team;
        team_list.push_back(team);
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
        problem_names.clear();
        for (int i = 0; i < problem_count; i++) {
            problem_names.push_back(string(1, 'A' + i));
        }

        // Initialize scoreboard with teams in lexicographic order
        scoreboard = team_list;
        sort(scoreboard.begin(), scoreboard.end(),
             [](const shared_ptr<Team>& a, const shared_ptr<Team>& b) {
                 return a->name < b->name;
             });

        cout << "[Info]Competition starts.\n";
    }

    void handle_submit(const string& problem_name, const string& team_name,
                      const string& status_str, int time) {
        if (teams.find(team_name) == teams.end()) {
            // Team doesn't exist, ignore (shouldn't happen per problem constraints)
            return;
        }

        JudgeStatus status = stringToStatus(status_str);

        auto& team = teams[team_name];
        team->add_submission(problem_name, status, time, is_frozen);

        // No output for submit command
    }

    void handle_flush() {
        update_scoreboard();
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

        // Output scoreboard before scrolling
        output_scoreboard();

        // TODO: Implement actual scrolling logic
        // This is a complex operation that involves unfreezing problems
        // and updating rankings

        // For now, just unfreeze everything
        is_frozen = false;
        for (auto& team : team_list) {
            for (auto& problem : team->problems) {
                problem.second.frozen_submissions = 0;
                problem.second.frozen_wrong_attempts = 0;
            }
        }

        // Update scoreboard after scrolling
        update_scoreboard();
        output_scoreboard();
    }

    void handle_query_ranking(const string& team_name) {
        if (teams.find(team_name) == teams.end()) {
            cout << "[Error]Query ranking failed: cannot find the team.\n";
            return;
        }

        cout << "[Info]Complete query ranking.\n";
        if (is_frozen) {
            cout << "[Warning]Scoreboard is frozen. The ranking may be inaccurate until it were scrolled.\n";
        }

        // Find team ranking
        int ranking = 1;
        for (const auto& team : scoreboard) {
            if (team->name == team_name) {
                cout << team_name << " NOW AT RANKING " << ranking << "\n";
                return;
            }
            ranking++;
        }
    }

    void handle_query_submission(const string& team_name, const string& problem_name,
                                const string& status_str) {
        if (teams.find(team_name) == teams.end()) {
            cout << "[Error]Query submission failed: cannot find the team.\n";
            return;
        }

        cout << "[Info]Complete query submission.\n";

        auto& team = teams[team_name];
        const auto& submissions = team->submissions;

        // Find the last matching submission
        for (auto it = submissions.rbegin(); it != submissions.rend(); ++it) {
            const auto& sub = *it;

            bool problem_match = (problem_name == "ALL" || sub.problem_name == problem_name);
            bool status_match = (status_str == "ALL" || statusToString(sub.status) == status_str);

            if (problem_match && status_match) {
                cout << team_name << " " << sub.problem_name << " "
                     << statusToString(sub.status) << " " << sub.time << "\n";
                return;
            }
        }

        cout << "Cannot find any submission.\n";
    }

    void handle_end() {
        cout << "[Info]Competition ends.\n";
    }

private:
    void update_scoreboard() {
        // Update ranking stats for all teams
        for (auto& team : team_list) {
            team->solved_count = 0;
            team->total_penalty = 0;
            team->solve_times.clear();

            for (const auto& problem : team->problems) {
                if (problem.second.solved && !problem.second.is_frozen()) {
                    team->solved_count++;
                    team->total_penalty += problem.second.penalty_time();
                    team->solve_times.push_back(problem.second.first_solve_time);
                }
            }

            sort(team->solve_times.begin(), team->solve_times.end(), greater<int>());
        }

        // Sort scoreboard according to ranking rules
        sort(scoreboard.begin(), scoreboard.end(),
             [](const shared_ptr<Team>& a, const shared_ptr<Team>& b) {
                 // 1. More solved problems ranks higher
                 if (a->solved_count != b->solved_count) {
                     return a->solved_count > b->solved_count;
                 }

                 // 2. Less penalty time ranks higher
                 if (a->total_penalty != b->total_penalty) {
                     return a->total_penalty < b->total_penalty;
                 }

                 // 3. Compare solve times (largest first, then second largest, etc.)
                 for (size_t i = 0; i < min(a->solve_times.size(), b->solve_times.size()); i++) {
                     if (a->solve_times[i] != b->solve_times[i]) {
                         return a->solve_times[i] < b->solve_times[i];
                     }
                 }

                 // If all solve times are equal up to min size, team with more solved problems ranks higher
                 if (a->solve_times.size() != b->solve_times.size()) {
                     return a->solve_times.size() > b->solve_times.size();
                 }

                 // 4. Lexicographic order of team names
                 return a->name < b->name;
             });
    }

    void output_scoreboard() {
        int ranking = 1;
        for (const auto& team : scoreboard) {
            cout << team->name << " " << ranking << " "
                 << team->solved_count << " " << team->total_penalty;

            for (const auto& problem_name : problem_names) {
                cout << " " << team->get_problem_display(problem_name);
            }
            cout << "\n";
            ranking++;
        }
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