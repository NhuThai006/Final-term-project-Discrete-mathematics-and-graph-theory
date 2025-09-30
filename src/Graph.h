#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <QString>
#include <QPointF>

struct Vertex {
    int id;
    QString name;
    QPointF position; // for GUI placement
};

struct Edge {
    int id;
    int u;
    int v;
    double weight; // e.g., length
    bool directed{false};
};

class Graph {
public:
    int addVertex(const QPointF &pos, const QString &name = {});
    int addEdge(int u, int v, double weight = 1.0, bool directed = false);
    void clear();
    void removeVertex(int vertexId);
    void removeEdge(int edgeId);

    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<Edge>& getEdges() const { return edges; }

    std::vector<int> neighbors(int u) const; // returns neighbor vertex ids (for undirected)
    int degree(int u) const;
    bool isConnectedUndirected() const;
    void dfs(int startVertex, std::vector<bool>& visited) const; // th�m khai b�o n�y
    bool isConnectedDirected() const; // th�m khai b�o n�y

    // adjacency list by vertex id -> edge indices
    const std::unordered_map<int, std::vector<int>>& adjacency() const { return vertexToEdgeIds; }

private:
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::unordered_map<int, std::vector<int>> vertexToEdgeIds;

    bool hasEdge(int u, int v) const {
        if (u < 0 || v < 0 || static_cast<size_t>(u) >= vertices.size() || static_cast<size_t>(v) >= vertices.size()) 
            return false;
        
        // Check both directions since the graph is undirected
        for (int edgeId : vertexToEdgeIds.at(u)) {
            const Edge& e = edges[edgeId];
            if ((e.u == u && e.v == v) || (e.u == v && e.v == u)) {
                return true;
            }
        }
        return false;
    }
};




