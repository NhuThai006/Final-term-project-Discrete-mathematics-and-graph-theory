#pragma once

#include "Graph.h"
#include <vector>
#include <optional>

struct EulerResult {
    std::vector<int> edgeOrder;
    bool isCycle{false};
    std::vector<int> vertexOrder;  // Th�m ?? l?u th? t? ??nh
};

namespace Algorithms {

namespace detail {
    extern std::optional<EulerResult> lastEulerResult;
    extern std::optional<EulerResult> lastPostmanResult;
    extern std::vector<int> highlightedEdges;  // L?u c�c c?nh ???c t� m�u
}

// Returns nullopt if no Euler path/cycle exists
std::optional<EulerResult> findEulerTourHierholzer(const Graph &graph);

// For Chinese Postman
std::optional<EulerResult> approximateChinesePostman(const Graph &graph);

// Utility shortest path
std::vector<int> shortestPathVertices(const Graph &graph, int source, int target);

// Summary access
std::optional<EulerResult> getEulerSummary();
std::optional<EulerResult> getPostmanSummary();
void clearResults();

// Highlight management
const std::vector<int>& getHighlightedEdges();
void clearHighlights();

// Vertex sequence helpers
std::vector<int> getVertexSequence(const Graph& graph, const std::vector<int>& edgeOrder);
}


