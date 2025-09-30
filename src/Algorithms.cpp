#include "Algorithms.h"
#include <queue>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>

using namespace std;

// Initialize static storage
namespace Algorithms::detail {
    std::optional<EulerResult> lastEulerResult;
    std::optional<EulerResult> lastPostmanResult;
}

namespace {
struct EdgeUse { int id; bool used{false}; };

bool isEulerianOrSemi(const Graph &g, bool &isCycle, int &startVertex) {
    if (!g.isConnectedUndirected()) return false;
    int oddCount = 0;
    startVertex = 0;
    for (const auto &v : g.getVertices()) {
        int d = g.degree(v.id);
        if (d % 2 == 1) {
            oddCount++;
            startVertex = v.id;
        } else if (startVertex == 0 && d > 0) {
            startVertex = v.id;
        }
    }
    if (oddCount == 0) { isCycle = true; return true; }
    if (oddCount == 2) { isCycle = false; return true; }
    return false;
}
}

optional<EulerResult> Algorithms::findEulerTourHierholzer(const Graph &graph) {
    bool isCycle = false; int start = 0;
    if (!isEulerianOrSemi(graph, isCycle, start)) {
        detail::lastEulerResult = nullopt;
        return nullopt;
    }

    // Build adjacency list and edge usage tracking
    unordered_map<int, vector<int>> adj = graph.adjacency();
    vector<bool> edgeUsed(graph.getEdges().size(), false);
    vector<int> path; // Final path as edge IDs
    
    // DFS function to find Euler path/cycle
    function<void(int)> dfs = [&](int u) {
        auto it = adj.find(u);
        if (it == adj.end()) return;
        
        // Create a copy of edges to iterate through
        vector<int> edges = it->second;
        for (int eid : edges) {
            if (!edgeUsed[eid]) {
                edgeUsed[eid] = true;
                const Edge &e = graph.getEdges()[eid];
                int v = (e.u == u) ? e.v : e.u;
                dfs(v);
                path.push_back(eid);
            }
        }
    };
    
    // Start DFS from the starting vertex
    dfs(start);
    
    // Reverse to get correct order
    reverse(path.begin(), path.end());
    
    // Verify all edges are used exactly once
    for (int i = 0; i < (int)graph.getEdges().size(); ++i) {
        if (!edgeUsed[i]) {
            return nullopt; // Some edges weren't used
        }
    }
    
    // Additional verification: check that each edge appears exactly once in path
    vector<int> edgeCount(graph.getEdges().size(), 0);
    for (int eid : path) {
        edgeCount[eid]++;
        if (edgeCount[eid] > 1) {
            return nullopt; // Edge used more than once
        }
    }
    
    EulerResult res;
    res.edgeOrder = move(path);
    res.isCycle = isCycle;
    
    detail::lastEulerResult = res;
    return detail::lastEulerResult;
}

vector<int> Algorithms::shortestPathVertices(const Graph &graph, int source, int target) {
    const int n = (int)graph.getVertices().size();
    vector<double> dist(n, numeric_limits<double>::infinity());
    vector<int> parent(n, -1);
    using QN = pair<double,int>;
    priority_queue<QN, vector<QN>, greater<QN>> pq;
    dist[source] = 0.0; pq.push({0.0, source});
    unordered_map<int, vector<int>> v2e = graph.adjacency();

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue;
        if (u == target) break;
        auto it = v2e.find(u);
        if (it == v2e.end()) continue;
        for (int eid : it->second) {
            const Edge &e = graph.getEdges()[eid];
            int w = (e.u == u ? e.v : e.u);
            double nd = d + e.weight;
            if (nd < dist[w]) { dist[w] = nd; parent[w] = u; pq.push({nd, w}); }
        }
    }
    vector<int> path;
    if (parent[target] == -1 && source != target) return path;
    for (int v = target; v != -1; v = parent[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

optional<EulerResult> Algorithms::approximateChinesePostman(const Graph &graph) {
    bool cycle = false; int start = 0;
    if (isEulerianOrSemi(graph, cycle, start) && cycle) {
        auto result = findEulerTourHierholzer(graph);
        detail::lastPostmanResult = result;
        return result;
    }

    // Collect odd-degree vertices
    vector<int> odd;
    for (const auto &v : graph.getVertices()) {
        if (graph.degree(v.id) % 2 == 1) odd.push_back(v.id);
    }
    if (odd.empty()) return findEulerTourHierholzer(graph);

    // Greedy pairing: repeatedly pair closest odd vertices by shortest path length
    vector<bool> used(graph.getVertices().size(), false);
    vector<pair<int,int>> pairs;
    unordered_set<int> oddSet(odd.begin(), odd.end());

    vector<int> remaining = odd;
    while (!remaining.empty()) {
        int a = remaining.back(); remaining.pop_back();
        double best = numeric_limits<double>::infinity();
        int bestIdx = -1; int bestV = -1;
        for (int i = 0; i < (int)remaining.size(); ++i) {
            int b = remaining[i];
            auto path = shortestPathVertices(graph, a, b);
            if (path.empty()) continue;
            double len = 0.0;
            for (size_t k = 1; k < path.size(); ++k) {
                // sum weights along edges
                // find edge between path[k-1], path[k]
                int u = path[k-1], v = path[k];
                double w = numeric_limits<double>::infinity();
                for (int eid : graph.adjacency().at(u)) {
                    const Edge &e = graph.getEdges()[eid];
                    int wv = (e.u == u ? e.v : e.u);
                    if (wv == v) { w = e.weight; break; }
                }
                len += w;
            }
            if (len < best) { best = len; bestIdx = i; bestV = b; }
        }
        if (bestIdx == -1) break;
        pairs.push_back({a, bestV});
        remaining.erase(remaining.begin() + bestIdx);
    }

    // Build a multigraph by duplicating edges along these shortest paths
    Graph augmented = graph;
    for (auto [a, b] : pairs) {
        auto path = shortestPathVertices(graph, a, b);
        for (size_t k = 1; k < path.size(); ++k) {
            int u = path[k-1], v = path[k];
            // find a representative edge and duplicate it in augmented
            int representative = -1; double w = 1.0;
            for (int eid : graph.adjacency().at(u)) {
                const Edge &e = graph.getEdges()[eid];
                int wv = (e.u == u ? e.v : e.u);
                if (wv == v) { representative = eid; w = e.weight; break; }
            }
            if (representative != -1) {
                augmented.addEdge(u, v, w, false);
            }
        }
    }

    auto result = findEulerTourHierholzer(augmented);
    detail::lastPostmanResult = result;
    return result;
}

optional<EulerResult> Algorithms::getEulerSummary() {
    return detail::lastEulerResult;
}

optional<EulerResult> Algorithms::getPostmanSummary() {
    return detail::lastPostmanResult;
}

void Algorithms::clearResults() {
    detail::lastEulerResult = nullopt;
    detail::lastPostmanResult = nullopt;
}


