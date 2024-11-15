#include <climits>
#include <numeric>
#include <stdexcept>

#include "tspsolver.hpp"

#define MSG_ERR_IRR_ARG         "costs size does not meet the required size"

#define MSG_ERR_IRR_COST        "irregular cost"

#define MSG_ERR_COST_TOO_LARGE  "cost is too large"

#define MSG_ERR_COST_TOO_SMALL  "cost is too small"

using namespace std;

const int TspSolver::NIL = INT_MAX >> 8;

class Solver {
public:
    Solver(vector<vector<int>> costs) : pOriginalCosts(costs) {
        assertCosts(costs);
        initOptPath(costs.size());

        vector<int> rowTitle, colTitle;
        rowTitle = colTitle = range(costs.size());

        solve(rowTitle, colTitle, costs);
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
    struct Arc {
        size_t row;
        size_t col;
        Arc(const size_t r, const size_t c) : row(r), col(c) {}
    };

    struct ArcInfo {
        size_t row;
        size_t col;
        int opportunityCost;
        ArcInfo(const int o) : opportunityCost(o) {}
        ArcInfo(const size_t r, const size_t c, const int o) : row(r), col(c), opportunityCost(o) {}
    };

    const int pMaxCost = INT_MAX >> 16;
    const int pMinCost = 0;

    int pUpperBound = INT_MAX;
    vector<vector<int>> pOriginalCosts;
    vector<int> pOptTarget;
    vector<int> pCurrOptTarget;
    vector<int> pCurrGroup;
    vector<Arc> zeroCostArcs;

    void assertCosts(vector<vector<int>> & costs) const {
        if (costs.size() <= 1)
            throw invalid_argument(MSG_ERR_IRR_COST);

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

    void solve(
        vector<int> & rowTitle,
        vector<int> & colTitle,
        vector<vector<int>> & costs,
        int lowerBound = 0
    ) {
        if (lowerBound >= pUpperBound)
            return;

        lowerBound += extractLowerBound(costs);

        if (lowerBound >= pUpperBound)
            return;

        ArcInfo bestArcInfo { searchBestAdvantageArc(costs) };

        setCurrGroup();

        const bool arcSelectable
            { isArcSelectable(rowTitle[bestArcInfo.row], colTitle[bestArcInfo.col]) };

        const bool arcSkippable
            { isArcSkippable(rowTitle[bestArcInfo.row], colTitle[bestArcInfo.col]) };

        if (arcSelectable) {
            vector<int> chooseArcRowTitle(costs.size() - 1);
            vector<int> chooseArcColTitle(costs.size() - 1);
            vector<vector<int>> chooseArcCosts(costs.size() - 1, vector<int>(costs.size() - 1));

            chooseArc(
                bestArcInfo,
                rowTitle,
                colTitle,
                costs,
                chooseArcRowTitle,
                chooseArcColTitle,
                chooseArcCosts
            );

            if (chooseArcCosts.size() == 1) {
                if (lowerBound + chooseArcCosts[0][0] >= pUpperBound)
                    return;

                pUpperBound = lowerBound + chooseArcCosts[0][0];
                pOptTarget = pCurrOptTarget;
                pOptTarget[rowTitle[bestArcInfo.row]] = colTitle[bestArcInfo.col];
                pOptTarget[chooseArcRowTitle[0]] = chooseArcColTitle[0];
                return;
            } else {
                pCurrOptTarget[rowTitle[bestArcInfo.row]] = colTitle[bestArcInfo.col];
                solve(chooseArcRowTitle, chooseArcColTitle, chooseArcCosts, lowerBound);
                pCurrOptTarget[rowTitle[bestArcInfo.row]] = tsp::NIL;
            }
        }

        if (arcSkippable) {
            const int originalArcCost { costs[bestArcInfo.row][bestArcInfo.col] };
            costs[bestArcInfo.row][bestArcInfo.col] = tsp::NIL;
            solve(rowTitle, colTitle, costs, lowerBound);
            costs[bestArcInfo.row][bestArcInfo.col] = originalArcCost;
        }

    }

    int extractLowerBound(vector<vector<int>> & costs) {
        const size_t size { costs.size() };
        int lowerBound { 0 };
        int minCost;

        zeroCostArcs.clear();

        for (size_t i { 0 }; i != size; i += 1) {
            minCost = INT_MAX;

            for (size_t j { 0 }; j != size; j += 1) {
                if (!costs[i][j])
                    zeroCostArcs.push_back(Arc(i, j));
                if (costs[i][j] < minCost)
                    minCost = costs[i][j];
            }

            if (!minCost)
                continue;

            for (size_t j { 0 }; j != size; j += 1) {
                if (costs[i][j] == minCost)
                    zeroCostArcs.push_back(Arc(i, j));
                costs[i][j] -= minCost;
            }

            lowerBound += minCost;
        }

        for (size_t j { 0 }; j != size; j += 1) {
            minCost = INT_MAX;

            for (size_t i { 0 }; i != size; i += 1) {
                if (costs[i][j] < minCost)
                    minCost = costs[i][j];
            }

            if (!minCost)
                continue;

            for (size_t i { 0 }; i != size; i += 1) {
                if (costs[i][j] == minCost)
                    zeroCostArcs.push_back(Arc(i, j));
                costs[i][j] -= minCost;
            }

            lowerBound += minCost;
        }

        return lowerBound;
    }

    ArcInfo searchBestAdvantageArc(const vector<vector<int>> & costs) const {
        const size_t size { costs.size() };
        ArcInfo bestArcInfo(INT_MIN);
        int minRowCost;
        int minColCost;
        size_t i, j;

        for (const Arc & arc : zeroCostArcs) {
            minColCost = INT_MAX;

            for (i = 0; i != size; i += 1) {
                if (i == arc.row)
                    continue;
                if (costs[i][arc.col] > pMaxCost)
                    continue;
                if (costs[i][arc.col] < minColCost)
                    minColCost = costs[i][arc.col];
            }

            minRowCost = INT_MAX;

            for (j = 0; j != size; j += 1) {
                if (j == arc.col)
                    continue;
                if (costs[arc.row][j] > pMaxCost)
                    continue;
                if (costs[arc.row][j] < minRowCost)
                    minRowCost = costs[arc.row][j];
            }

            if (minColCost + minRowCost > bestArcInfo.opportunityCost) {
                bestArcInfo.row = arc.row;
                bestArcInfo.col = arc.col;
                bestArcInfo.opportunityCost = minColCost + minRowCost;
            }
        }

        return bestArcInfo;
    }

    void chooseArc(
        const ArcInfo & bestArcInfo,
        const vector<int> & rowTitle,
        const vector<int> & colTitle,
        const vector<vector<int>> & costs,
        vector<int> & chooseArcRowTitle,
        vector<int> & chooseArcColTitle,
        vector<vector<int>> & chooseArcCosts
    ) const {
        size_t size { costs.size() };
        size_t i, j, p, q;

        for (i = 0, p = 0; i != size; i += 1) {
            if (i != bestArcInfo.row)
                chooseArcRowTitle[p++] = rowTitle[i];
        }

        for (i = 0, p = 0; i != size; i += 1) {
            if (i != bestArcInfo.col)
                chooseArcColTitle[p++] = colTitle[i];
        }

        for (i = 0, p = 0; i != size; i += 1) {
            if (i != bestArcInfo.row) {
                for (j = 0, q = 0; j != size; j += 1) {
                    if (j != bestArcInfo.col)
                        chooseArcCosts[p][q++] = costs[i][j];
                }

                p += 1;
            }
        }
    }

    void setCurrGroup() {
        pCurrGroup = range(pCurrOptTarget.size());
        int size { static_cast<int>(pCurrOptTarget.size()) };
        int minIdx, maxIdx, minGroup, maxGroup;

        for (int i { 0 }; i != size; i += 1) {
            if (pCurrOptTarget[i] == tsp::NIL)
                continue;

            minGroup = findGroup(minIdx = min(i, pCurrOptTarget[i]));
            maxGroup = findGroup(maxIdx = max(i, pCurrOptTarget[i]));

            if (minGroup > maxGroup)
                swap(minGroup, maxGroup);

            pCurrGroup[maxGroup] = minGroup;
        }
    }

    bool isArcSelectable(const int from, const int to) {
        if (pCurrOptTarget[from] != tsp::NIL)
            return false;

        for (const int t : pCurrOptTarget) {
            if (t == to)
                return false;
        }

        return findGroup(from) != findGroup(to);
    }

    bool isArcSkippable(const int from, const int to) {
        int size { static_cast<int>(pCurrOptTarget.size()) };
        vector<int> group(size);
        int i;

        for (i = 0; i != size; i += 1)
            group[i] = findGroup(i);

        for (i = 0; i != size; i += 1) {
            if (i == from)
                continue;
            if (i == to)
                continue;
            if (group[i] == group[from])
                continue;
            if (group[i] == group[to])
                continue;
            return true;
        }

        return false;
    }

    int findGroup(int index) const {
        while (pCurrGroup[index] != index)
            index = pCurrGroup[index];
        return index;
    }

    vector<int> range(const size_t size) const {
        vector<int> v(size);
        iota(v.begin(), v.end(), 0);
        return v;
    }

};

int TspSolver::solve(const vector<vector<int>> & costs, vector<int> & cyclePath, int & cost) const {
    Solver solver(costs);
    cyclePath = solver.getCycle();
    cost = solver.getCost();
    return solver.isValid() ? 0 : -1;
}
