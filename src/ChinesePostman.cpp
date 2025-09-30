#include "Algorithms.h"
#include "ChinesePostman.h"
#include <queue>
#include <limits>
#include <algorithm>
#include <map>

using namespace std;

// Dijkstra trả về đường đi ngắn nhất từ s đến mọi đỉnh
static vector<double> dijkstra(const Graph& g, int s, vector<int>& prev) {
    const auto& verts = g.getVertices();
    const auto& edges = g.getEdges();
    vector<double> dist(verts.size(), numeric_limits<double>::infinity());
    prev.assign(verts.size(), -1);
    dist[s] = 0;
    using P = pair<double, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    pq.emplace(0, s);
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (int eid : g.adjacency().at(u)) {
            const auto& e = edges[eid];
            int v = (e.u == u) ? e.v : e.u;
            double w = e.weight;
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                pq.emplace(dist[v], v);
            }
        }
    }
    return dist;
}

// Sinh tất cả các matching giữa các đỉnh lẻ, trả về matching có tổng trọng số nhỏ nhất
static void minWeightMatching(const vector<vector<double>>& cost, vector<pair<int,int>>& bestMatching, double& minCost) {
    int n = cost.size();
    vector<int> idx(n);
    for (int i = 0; i < n; ++i) idx[i] = i;
    minCost = numeric_limits<double>::infinity();
    vector<pair<int,int>> matching;
    function<void(int, double)> dfs = [&](int i, double acc) {
        if (i == n) {
            if (acc < minCost) {
                minCost = acc;
                bestMatching = matching;
            }
            return;
        }
        if (idx[i] == -1) { dfs(i+1, acc); return; }
        for (int j = i+1; j < n; ++j) {
            if (idx[j] == -1) continue;
            matching.emplace_back(idx[i], idx[j]);
            int a = idx[i], b = idx[j];
            int oldi = idx[i], oldj = idx[j];
            idx[i] = idx[j] = -1;
            dfs(i+1, acc + cost[a][b]);
            idx[i] = oldi; idx[j] = oldj;
            matching.pop_back();
        }
    };
    dfs(0, 0);
}

// Truy vết đường đi ngắn nhất từ prev
static vector<int> reconstructPath(int from, int to, const vector<int>& prev) {
    vector<int> path;
    for (int v = to; v != from; v = prev[v]) {
        if (v == -1) return {};
        path.push_back(v);
    }
    path.push_back(from);
    reverse(path.begin(), path.end());
    return path;
}

ChinesePostmanResult ChinesePostmanOptimal::solve(const Graph& g) {
    ChinesePostmanResult result;
    const auto& verts = g.getVertices();
    const auto& edges = g.getEdges();
    // 1. Tìm các đỉnh bậc lẻ
    vector<int> odd;
    for (size_t i = 0; i < verts.size(); ++i) {
        if (g.degree(static_cast<int>(i)) % 2 == 1) odd.push_back(static_cast<int>(i));
    }
    if (odd.empty()) {
        // Đã Eulerian, chỉ cần tìm Euler circuit
        auto eulerRes = Algorithms::findEulerTourHierholzer(g);
        if (eulerRes) {
            result.edgeOrder = eulerRes->edgeOrder;
            result.isCycle = eulerRes->isCycle;
        }
        return result;
    }
    // 2. Tìm đường đi ngắn nhất giữa các đỉnh lẻ
    int n = odd.size();
    vector<vector<double>> cost(n, vector<double>(n, 1e9));
    vector<vector<vector<int>>> paths(n, vector<vector<int>>(n));
    for (int i = 0; i < n; ++i) {
        vector<int> prev;
        auto dist = dijkstra(g, odd[i], prev);
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                cost[i][j] = dist[odd[j]];
                paths[i][j] = reconstructPath(odd[i], odd[j], prev);
            }
        }
    }
    // 3. Ghép cặp tối ưu
    vector<pair<int,int>> matching;
    double minCost = 0;
    minWeightMatching(cost, matching, minCost);
    // 4. Tạo multigraph mới: thêm các đường đi duplicate
    struct MultiEdge { int u, v, id; double weight; };
    vector<MultiEdge> multiEdges;
    for (const auto& e : edges) {
        multiEdges.push_back({e.u, e.v, e.id, e.weight});
    }
    map<pair<int,int>, int> edgeDup;
    map<int, vector<int>> dupPath; // lưu mapping duplicate edge -> path gốc
    int nextId = static_cast<int>(edges.size());
    for (auto& p : matching) {
        const auto& path = paths[p.first][p.second];
        for (size_t i = 1; i < path.size(); ++i) {
            int u = path[i-1], v = path[i];
            if (u > v) std::swap(u, v);
            // thêm 1 cạnh duplicate với id mới
            int dupId = nextId++;
            multiEdges.push_back({u, v, dupId, 1.0});
            // lưu đường đi thật (danh sách id cạnh gốc) vào dupPath
            vector<int> realEdges;
            for (const auto& e : edges) {
                if ((e.u == path[i-1] && e.v == path[i]) ||
                    (e.v == path[i-1] && e.u == path[i])) {
                    realEdges.push_back(e.id);
                    break;
                }
            }
            dupPath[dupId] = realEdges;
        }
    }
    // 5. Tìm Euler circuit trên multigraph
    map<int, vector<pair<int,int>>> adj; // u -> list of (v, edgeIdx)
    for (size_t i = 0; i < multiEdges.size(); ++i) {
        const auto& e = multiEdges[i];
        adj[e.u].emplace_back(e.v, static_cast<int>(i));
        adj[e.v].emplace_back(e.u, static_cast<int>(i));
    }
    vector<bool> used(multiEdges.size(), false);
    vector<int> circuit;
    int startVertex = odd.empty() ? 0 : odd[0];
    function<void(int)> dfs = [&](int u) {
        while (!adj[u].empty()) {
            auto [v, eid] = adj[u].back(); adj[u].pop_back();
            if (used[eid]) continue;
            used[eid] = true;
            dfs(v);
            circuit.push_back(multiEdges[eid].id);
        }
    };
    dfs(startVertex);
    reverse(circuit.begin(), circuit.end());
    // 6. Lưu lại thứ tự id cạnh (bao gồm cả cạnh duplicate)
    for (int eid : circuit) {
        result.edgeOrder.push_back(eid);
    }
    result.isCycle = true;
    return result;
}
