/// Copyright (c) 2023 Araf Al Jami
#include <cpr/cpr.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <random>
#undef NDEBUG
#include <cassert>

struct Problem
{
    std::string name;
    int contestId;
    std::string index;
    int rating;
    bool operator<(const Problem &other) const
    {
        if (rating == other.rating)
        {
            if (contestId == other.contestId)
            {
                return index < other.index;
            }
            return contestId < other.contestId;
        }
        return rating < other.rating;
    }
};

std::set<Problem> problems;
std::vector<Problem> unsolved;
std::string username = "CLown1331";
int ratingLowerBound = 800;
int ratingUpperBound = 1300;

int fetchProblems()
{
    cpr::Response r = cpr::Get(cpr::Url{"https://codeforces.com/api/problemset.problems"});
    if (r.status_code != 200)
    {
        return -1;
    }

    auto json = nlohmann::json::parse(r.text);
    if (json["status"] != "OK")
    {
        return -1;
    }

    if (json["result"].empty())
    {
        return -1;
    }

    for (auto &problem : json["result"]["problems"])
    {
        if (problem["rating"].is_null())
        {
            continue;
        }
        problems.insert(Problem{problem["name"], problem["contestId"], problem["index"], problem["rating"]});
    }

    return 0;
}

int processFetchedProblem()
{
    cpr::Response r = cpr::Get(cpr::Url{"https://codeforces.com/api/user.status?handle=" + username});
    if (r.status_code != 200)
    {
        return -1;
    }

    auto json = nlohmann::json::parse(r.text);
    if (json["status"] != "OK")
    {
        return -1;
    }

    if (json["result"].empty())
    {
        return -1;
    }

    for (auto &submission : json["result"])
    {
        if (submission["verdict"] != "OK")
        {
            continue;
        }
        if (submission["problem"]["rating"].is_null())
        {
            continue;
        }
        problems.erase(Problem{
            submission["problem"]["name"],
            submission["problem"]["contestId"],
            submission["problem"]["index"],
            submission["problem"]["rating"]});
    }

    unsolved = std::vector<Problem>(problems.begin(), problems.end());

    std::sort(unsolved.begin(), unsolved.end());

    return 0;
}

int processOpts(int argc, char **argv)
{
    int c;
    while ((c = getopt(argc, argv, "l:h:u:")) != -1)
    {
        switch (c)
        {
        case 'l':
            ratingLowerBound = std::stoi(optarg);
            break;
        case 'u':
            ratingUpperBound = std::stoi(optarg);
            break;
        case 'h':
            username = optarg;
            break;
        default:
            break;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    const clock_t tStart = clock();

    int rv;

    rv = processOpts(argc, argv);

    assert(rv == 0 && "Failed to process options");

    assert(ratingLowerBound <= ratingUpperBound && "Lower bound must be less than or equal to upper bound");

    rv = fetchProblems();

    assert(rv == 0 && "Failed to fetch problems");

    rv = processFetchedProblem();

    assert(rv == 0 && "Failed to process fetched problems");

    int lb = std::lower_bound(unsolved.begin(), unsolved.end(), Problem{"", 0, "", ratingLowerBound - 1}) - unsolved.begin();

    int ub = std::upper_bound(unsolved.begin(), unsolved.end(), Problem{"", 0, "", ratingUpperBound + 1}) - unsolved.begin();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(lb, ub);

    int choice = distrib(gen);

    Problem p = unsolved[choice];

    std::string problemUrl = "https://codeforces.com/contest/" + std::to_string(p.contestId) + "/problem/" + p.index;

    std::cout << "Looking for a problem in the range [" << ratingLowerBound << ", " << ratingUpperBound << "] for " << username << "\n";
    std::cout << p.contestId << p.index << " " << p.name << "\nRating: " << p.rating << "\n";

    double totalRuntime = static_cast<double>(clock() - tStart) / CLOCKS_PER_SEC;

    std::cout << "Runtime: " << std::setprecision(3) << totalRuntime << "s\n";

    rv = execl("/usr/bin/open", "-u", problemUrl.c_str(), NULL);

    assert(rv == 0 && "Failed to open browser");

    return 0;
}
