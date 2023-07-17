#include <cpr/cpr.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <random>
#include <cassert>

struct Problem {
    std::string name;
    int contestId;
    std::string index;
    int rating;
    bool operator<(const Problem& other) const
    {
        if (rating == other.rating) {
            if (contestId == other.contestId) {
                return index < other.index;
            }
            return contestId < other.contestId;
        }
        return rating < other.rating;
    }
};

std::set<Problem> problems;
std::vector <Problem> unsolved;

int fetchProblems()
{
    cpr::Response r = cpr::Get(cpr::Url{"https://codeforces.com/api/problemset.problems"});
    if (r.status_code != 200) {
        return -1;
    }

    auto json = nlohmann::json::parse(r.text);
    if (json["status"] != "OK") {
        return -1;
    }

    if (json["result"].empty()) {
        return -1;
    }

    for (auto& problem : json["result"]["problems"]) {
        if (problem["rating"].is_null()) {
            continue;
        }
        problems.insert(Problem{problem["name"], problem["contestId"], problem["index"], problem["rating"]});
    }

    return 0;
}

int main(int argc, char** argv)
{
    std::string username = "CLown1331";
    
    int rv = fetchProblems();

    assert(rv == 0 && "Failed to fetch problems");

    cpr::Response r = cpr::Get(cpr::Url{"https://codeforces.com/api/user.status?handle=" + username});
    if (r.status_code != 200) {
        return -1;
    }

    auto json = nlohmann::json::parse(r.text);
    if (json["status"] != "OK") {
        return -1;
    }

    if (json["result"].empty()) {
        return -1;
    }

    for (auto& submission : json["result"]) {
        if (submission["verdict"] != "OK") {
            continue;
        }
        if (submission["problem"]["rating"].is_null()) {
            continue;
        }
        problems.erase(Problem
        {
            submission["problem"]["name"],
            submission["problem"]["contestId"],
            submission["problem"]["index"],
            submission["problem"]["rating"]
        });
    }

    unsolved = std::vector<Problem>(problems.begin(), problems.end());

    std::sort(unsolved.begin(), unsolved.end());

    int lb = std::lower_bound(unsolved.begin(), unsolved.end(), Problem{"", 0, "", 799}) - unsolved.begin();

    int ub = std::upper_bound(unsolved.begin(), unsolved.end(), Problem{"", 0, "", 1301}) - unsolved.begin();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(lb, ub);

    int choice = distrib(gen);

    Problem p = unsolved[choice];

    std::string problemUrl = "https://codeforces.com/contest/" + std::to_string(p.contestId) + "/problem/" + p.index;

    std::cout << "Choice: " << p.contestId << p.index << " " << p.name << " " << p.rating << " rating" << "\n";

    rv  = execl("/usr/bin/open", "-u", problemUrl.c_str(), NULL);

    assert(rv == 0 && "Failed to open browser");
    
    return 0;
}
