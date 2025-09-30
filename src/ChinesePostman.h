#pragma once
#include "Graph.h"
#include <vector>
#include <utility>

struct ChinesePostmanResult {
    std::vector<int> edgeOrder; // Euler circuit edge ids (có th? có duplicate)
    bool isCycle = true;
};

namespace ChinesePostmanOptimal {
    // Tr? v? route Euler t?i ?u (edge ids, có th? duplicate) cho ?? th? g
    ChinesePostmanResult solve(const Graph& g);
}
