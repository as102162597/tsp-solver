#include <climits>
#include <stdexcept>

#include "tspsolver.hpp"

using namespace std;

#define MSG_ERR_IRR_ARG         "costs size does not meet the required size"

#define MSG_ERR_IRR_COST        "irregular cost"

#define MSG_ERR_COST_TOO_LARGE  "cost is too large"

#define MSG_ERR_COST_TOO_SMALL  "cost is too small"

class BruteForceSolver {
public:
    BruteForceSolver(vector<vector<int>> costs) : pOriginalCosts(costs) {
        assertCosts(costs);
        initOptPath(costs.size());
        solve();
    }

    int getCost() const {
        return pUpperBound;
    }

    bool isValid() const {
        for (size_t i { 0 }; i != pOptTarget.size(); i += 1) {
            if (pOriginalCosts[i][pOptTarget[i]] == tsp::NIL)
                return false;
        }
        return true;
    }

    vector<int> getCycle() const {
        vector<int> cycle(pOptTarget.size() + 1);
        int start { 0 };

        for (int & n : cycle) {
            n = start;
            start = pOptTarget[start];
        }

        return cycle;
    }

private:
    const int pMaxCost = INT_MAX >> 16;
    const int pMinCost = 0;

    int pUpperBound = INT_MAX;
    vector<vector<int>> pOriginalCosts;
    vector<int> pOptTarget;
    vector<int> pCurrOptTarget;

    void assertCosts(vector<vector<int>> & costs) const {
        for (size_t i { 0 }; i != costs.size(); i += 1) {
            if (costs[i].size() != costs.size()) {
                throw invalid_argument(MSG_ERR_IRR_ARG);
            } else if (costs[i][i] != tsp::NIL) {
                throw invalid_argument(MSG_ERR_IRR_COST);
            }

            for (size_t j { 0 }; j != costs[i].size(); j += 1) {
                if (costs[i][j] == tsp::NIL) {
                    continue;
                } else if (costs[i][j] > pMaxCost) {
                    throw out_of_range(MSG_ERR_COST_TOO_LARGE);
                } else if (costs[i][j] < pMinCost) {
                    throw out_of_range(MSG_ERR_COST_TOO_SMALL);
                }
            }
        }
    }

    void initOptPath(const size_t size) {
        pOptTarget = pCurrOptTarget = vector<int>(size, tsp::NIL);
    }

    void solve(const size_t from = 0, const int lowerBound = 0, const size_t pathLength = 0) {
        if (lowerBound >= pUpperBound)
            return;

        if (pathLength == pCurrOptTarget.size() - 1) {
            if (lowerBound + pOriginalCosts[from][0] < pUpperBound) {
                pUpperBound = lowerBound + pOriginalCosts[from][0];
                pOptTarget = pCurrOptTarget;
                pOptTarget[from] = 0;
            }
            return;
        }

        for (size_t to { 0 }; to != pCurrOptTarget.size(); to += 1) {
            if (to == from)
                continue;
            if (pCurrOptTarget[to] != tsp::NIL)
                continue;
            pCurrOptTarget[from] = static_cast<int>(to);
            solve(to, lowerBound + pOriginalCosts[from][to], pathLength + 1);
            pCurrOptTarget[from] = tsp::NIL;
        }
    }

};

int TspSolver::bruteForceSolve(const vector<vector<int>> & costs, vector<int> & cyclePath, int & cost) const {
    BruteForceSolver solver(costs);
    cyclePath = solver.getCycle();
    cost = solver.getCost();
    return solver.isValid() ? 0 : -1;
}
