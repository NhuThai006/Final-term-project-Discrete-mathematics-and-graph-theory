#pragma once
#include "Graph.h"
#include <vector>
#include <utility>

struct ChinesePostmanResult {
    std::vector<int> edgeOrder; // Euler circuit edge ids (c� th? c� duplicate)
    bool isCycle = true;
};

namespace ChinesePostmanOptimal {
    // Tr? v? route Euler t?i ?u (edge ids, c� th? duplicate) cho ?? th? g
    ChinesePostmanResult solve(const Graph& g);
}
