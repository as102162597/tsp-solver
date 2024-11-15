#ifndef __TSP_SOLVER__
#define __TSP_SOLVER__

#include <vector>

class TspSolver {
public:
    static const int NIL;

    /**
    * @brief Solves the Traveling Salesman Problem (TSP, find the shortest Hamilton cycle).
    *
    * This function is used to solve the Traveling Salesman Problem (TSP), finding the minimum-
    * cost tour where the salesman visits each city once and returns to the starting city. The
    * input is a cost matrix representing the path costs between cities, and the function will
    * update the minimum cost and its corresponding tour path. Cities that are unreachable can be
    * represented by tsp::NIL.
    *
    * The cost matrix `costs` must meet the following requirements:
    * - The size of matrix must be greater than 1.
    * - The cost matrix must be square.
    * - The diagonal elements of the cost matrix (costs[i][i]) must be tsp::NIL.
    * - The cost matrix must not contain negative values.
    * - The values in the cost matrix must not exceed 30000.
    *
    * @param costs      A 2D vector representing the cost matrix between cities, where
    *                   costs[i][j] is the cost of traveling from city i to city j.
    * @param cyclePath  An integer vector that stores the shortest tour path, represented as a
    *                   sequence of city indices in the order they are visited.
    * @param cost       An integer reference that will store the total cost of the shortest tour
    *                   path after the function is executed.
    * @return           Returns 0 if the shortest tour path is found successfully; returns -1 if
    *                   no feasible solution is found.
    */

    int solve(
        const std::vector<std::vector<int>> &   costs,
        std::vector<int> &                      cyclePath,
        int &                                   cost
    ) const;

    int bruteForceSolve(
        const std::vector<std::vector<int>> &   costs,
        std::vector<int> &                      cyclePath,
        int &                                   cost
    ) const;

};

using tsp = TspSolver;

#endif
