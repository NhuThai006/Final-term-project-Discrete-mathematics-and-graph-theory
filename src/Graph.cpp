#include "Graph.h"
#include <queue>

static QString indexToLetters(int index) {
    QString s;
    int i = index;
    do {
        int r = i % 26;
        s.prepend(QChar('A' + r));
        i = i / 26 - 1;
    } while (i >= 0);
    return s;
}

int Graph::addVertex(const QPointF &pos, const QString &name) {
    int id = static_cast<int>(vertices.size());
    QString label = name.isEmpty() ? indexToLetters(id) : name;
    vertices.push_back(Vertex{ id, label, pos });
    return id;
}

int Graph::addEdge(int u, int v, double weight, bool directed) {
    int id = static_cast<int>(edges.size());
    edges.push_back(Edge{ id, u, v, weight, directed });
    vertexToEdgeIds[u].push_back(id);
    vertexToEdgeIds[v].push_back(id);
    return id;
}

void Graph::clear() {
    vertices.clear();
    edges.clear();
    vertexToEdgeIds.clear();
}

void Graph::removeEdge(int edgeId) {
    if (edgeId < 0 || static_cast<size_t>(edgeId) >= edges.size()) return;
    Edge e = edges[edgeId];
    // mark removal by swapping with last and popping to keep ids contiguous? we keep ids stable; rebuild structures instead
    std::vector<Edge> newEdges;
    newEdges.reserve(edges.size());
    vertexToEdgeIds.clear();
    for (const auto &edge : edges) {
        if (edge.id == edgeId) continue;
        Edge copy = edge;
        copy.id = static_cast<int>(newEdges.size());
        newEdges.push_back(copy);
        vertexToEdgeIds[copy.u].push_back(copy.id);
        vertexToEdgeIds[copy.v].push_back(copy.id);
    }
    edges.swap(newEdges);
}

void Graph::removeVertex(int vertexId) {
    if (vertexId < 0 || static_cast<size_t>(vertexId) >= vertices.size()) return;
    // remove edges incident to vertex
    std::vector<Edge> filteredEdges;
    filteredEdges.reserve(edges.size());
    for (const auto &e : edges) {
        if (e.u == vertexId || e.v == vertexId) continue;
        filteredEdges.push_back(e);
    }
    // rebuild vertices with compact ids and remap edges
    std::vector<int> oldToNew(vertices.size(), -1);
    std::vector<Vertex> newVerts;
    newVerts.reserve(vertices.size() - 1);
    for (const auto &v : vertices) {
        if (v.id == vertexId) continue;
        Vertex nv = v;
        nv.id = static_cast<int>(newVerts.size());
        oldToNew[v.id] = nv.id;
        newVerts.push_back(nv);
    }
    std::vector<Edge> newEdges;
    newEdges.reserve(filteredEdges.size());
    vertexToEdgeIds.clear();
    for (const auto &e : filteredEdges) {
        Edge ne = e;
        ne.u = oldToNew[e.u];
        ne.v = oldToNew[e.v];
        ne.id = static_cast<int>(newEdges.size());
        newEdges.push_back(ne);
        vertexToEdgeIds[ne.u].push_back(ne.id);
        vertexToEdgeIds[ne.v].push_back(ne.id);
    }
    vertices.swap(newVerts);
    edges.swap(newEdges);
}

std::vector<int> Graph::neighbors(int u) const {
    std::vector<int> nbs;
    auto it = vertexToEdgeIds.find(u);
    if (it == vertexToEdgeIds.end()) return nbs;
    for (int eid : it->second) {
        const Edge &e = edges[eid];
        int w = (e.u == u ? e.v : e.u);
        nbs.push_back(w);
    }
    return nbs;
}

int Graph::degree(int u) const {
    auto it = vertexToEdgeIds.find(u);
    if (it == vertexToEdgeIds.end()) return 0;
    return static_cast<int>(it->second.size());
}

bool Graph::isConnectedUndirected() const {
    if (vertices.empty()) return true;
    // Find a vertex with non-zero degree to start
    int start = -1;
    for (const auto &v : vertices) {
        if (degree(v.id) > 0) { start = v.id; break; }
    }
    if (start == -1) return true; // no edges

    std::vector<bool> visited(vertices.size(), false);
    std::queue<int> q;
    q.push(start);
    visited[start] = true;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int w : neighbors(u)) {
            if (!visited[w]) { visited[w] = true; q.push(w); }
        }
    }

    // Check all vertices with non-zero degree were visited
    for (const auto &v : vertices) {
        if (degree(v.id) > 0 && !visited[v.id]) return false;
    }
    return true;
}

void Graph::dfs(int startVertex, std::vector<bool>& visited) const {
    visited[startVertex] = true;
    // Process the vertex (for example, print it)
    // std::cout << "Visited: " << vertices[startVertex].label.toStdString() << std::endl;

    for (int neighbor : neighbors(startVertex)) {
        if (!visited[neighbor]) {
            dfs(neighbor, visited);
        }
    }
}

bool Graph::isConnectedDirected() const {
    if (vertices.empty()) return true;

    std::vector<bool> visited(vertices.size(), false);
    int startVertex = 0; // Let's start with vertex 0

    dfs(startVertex, visited);

    // Check if all vertices were visited
    for (bool v : visited) {
        if (!v) return false;
    }
    return true;
}


