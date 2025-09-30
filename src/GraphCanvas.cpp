#include "GraphCanvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

static constexpr double VERTEX_RADIUS = 10.0;

// Convert 0-based index to alphabetical label: A, B, C, ..., Z, AA, AB, ...
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

GraphCanvas::GraphCanvas(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setAutoFillBackground(true);
}

void GraphCanvas::setRoute(const std::vector<int>& edgeOrder) {
    routeEdgeOrder = edgeOrder;
    update();
}

void GraphCanvas::clearRoute() {
    routeEdgeOrder.clear();
    update();
}

int GraphCanvas::hitTestVertex(const QPointF &p) const {
    for (const auto &v : graph.getVertices()) {
        if (QLineF(p, v.position).length() <= VERTEX_RADIUS + 3) return v.id;
    }
    return -1;
}

int GraphCanvas::hitTestEdge(const QPointF &p, int *outU, int *outV, double threshold) const {
    const auto &edges = graph.getEdges();
    const auto &verts = graph.getVertices();
    int foundId = -1;
    double bestDist = threshold;
    for (const auto &e : edges) {
        if (e.u < 0 || static_cast<size_t>(e.u) >= verts.size() || e.v < 0 || static_cast<size_t>(e.v) >= verts.size()) continue;
        QLineF line(verts[e.u].position, verts[e.v].position);
        // distance from point to segment
        QPointF a = line.p1();
        QPointF b = line.p2();
        QPointF ab = b - a;
        double ab2 = QPointF::dotProduct(ab, ab);
        if (ab2 <= 1e-6) continue;
        double t = QPointF::dotProduct(p - a, ab) / ab2;
        t = std::max(0.0, std::min(1.0, t));
        QPointF proj = a + ab * t;
        double dist = QLineF(p, proj).length();
        if (dist <= bestDist) {
            bestDist = dist;
            foundId = e.id;
            if (outU) *outU = e.u;
            if (outV) *outV = e.v;
        }
    }
    return foundId;
}

void GraphCanvas::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // draw background image if available
    if (!backgroundImage.isNull()) {
        painter.drawImage(rect(), backgroundImage);
    }

    // draw edges
    QPen edgePen(QColor(80,80,80)); edgePen.setWidth(2);
    painter.setPen(edgePen);
    for (const auto &e : graph.getEdges()) {
        const auto &u = graph.getVertices()[e.u];
        const auto &v = graph.getVertices()[e.v];
        painter.drawLine(u.position, v.position);
    }

    // draw route overlay
    if (!routeEdgeOrder.empty()) {
        QPen routePen(QColor(0,120,255)); routePen.setWidth(4);
        painter.setPen(routePen);
        const auto &edges = graph.getEdges();
        const auto &verts = graph.getVertices();
        for (int eid : routeEdgeOrder) {
            if (eid < 0 || static_cast<size_t>(eid) >= edges.size()) continue;
            const auto &e = edges[eid];
            if (e.u < 0 || static_cast<size_t>(e.u) >= verts.size() ||
                e.v < 0 || static_cast<size_t>(e.v) >= verts.size()) continue;
            const auto &u = verts[e.u];
            const auto &v = verts[e.v];
            painter.drawLine(u.position, v.position);
        }
    }

    // draw edge labels (1, 2, 3, ...) with larger font and slight offset above edge
    painter.setPen(QPen(Qt::black));
    QFont edgeFont = painter.font();
    edgeFont.setPointSizeF(std::max(15.0, edgeFont.pointSizeF() * 1.5));
    painter.setFont(edgeFont);
    for (const auto &e : graph.getEdges()) {
        const auto &u = graph.getVertices()[e.u];
        const auto &v = graph.getVertices()[e.v];
        QPointF mid((u.position.x() + v.position.x()) * 0.5, (u.position.y() + v.position.y()) * 0.5);
        QPointF d = v.position - u.position;
        double len = std::hypot(d.x(), d.y());
        QPointF n = (len > 0.0) ? QPointF(-d.y() / len, d.x() / len) : QPointF(0.0, -1.0);
        QPointF pos = mid + n * 10.0;
        QRectF r(pos.x() - 14, pos.y() - 14, 28, 28);
        painter.drawText(r, Qt::AlignCenter, QString::number(e.id + 1));
    }

    // draw vertices (circles) and their labels (A, B, C, ...)
    QFont vertexFont = painter.font();
    vertexFont.setPointSizeF(16);
    painter.setFont(vertexFont);
    for (const auto &v : graph.getVertices()) {
        // Draw vertex circle
        painter.setBrush(Qt::white);
        painter.setPen(QPen(Qt::black, 2));
        painter.drawEllipse(v.position, VERTEX_RADIUS, VERTEX_RADIUS);
        // Draw label (A, B, C, ...) ph�a tr�n ??nh
        painter.setPen(Qt::black);
        QString label = v.name.isEmpty() ? indexToLetters(v.id) : v.name;
        // D?ch label l�n tr�n h�nh tr�n, kh�ng ??ng v�o vertex
        QRectF textRect(v.position.x() - VERTEX_RADIUS, v.position.y() - VERTEX_RADIUS - 28, VERTEX_RADIUS*2, VERTEX_RADIUS*2);
        painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignBottom, label);
    }
}

void GraphCanvas::mousePressEvent(QMouseEvent *event) {
    QPointF pos = event->position();
    
    switch (mode) {
        case AddVertex: {
            graph.addVertex(pos);
            update();
            emit statusMessage("Added vertex");
            break;
        }
        case AddEdge: {
            int v = hitTestVertex(pos);
            if (v >= 0) {
                if (pendingEdgeFrom < 0) {
                    pendingEdgeFrom = v;
                    emit statusMessage("Select second vertex");
                } else if (v != pendingEdgeFrom) {  // Prevent self-loops
                    // Only add edge if it doesn't already exist
                    int edgeId = graph.addEdge(pendingEdgeFrom, v);
                    if (edgeId >= 0) {
                        emit statusMessage("Added edge");
                    } else {
                        emit statusMessage("Edge already exists");
                    }
                    pendingEdgeFrom = -1;
                    update();
                }
            }
            break;
        }
        case MoveVertex: {
            draggingVertex = hitTestVertex(pos);
            break;
        }
        case Eraser: {
            // Ưu tiên xóa đỉnh nếu click trúng đỉnh, ngược lại thử xóa cạnh gần nhất
            int vId = hitTestVertex(pos);
            if (vId >= 0) {
                graph.removeVertex(vId);
                clearRoute();
                update();
                emit statusMessage("Deleted vertex");
                break;
            }
            int u = -1, v = -1;
            int eId = hitTestEdge(pos, &u, &v);
            if (eId >= 0) {
                graph.removeEdge(eId);
                clearRoute();
                update();
                emit statusMessage("Deleted edge");
            }
            break;
        }
        case None: {
            // Kh�ng l�m g� c?
            break;
        }
    }
}

void GraphCanvas::mouseMoveEvent(QMouseEvent *ev) {
    if (mode == MoveVertex && draggingVertex != -1) {
        auto &verts = const_cast<std::vector<Vertex>&>(graph.getVertices());
        verts[draggingVertex].position = ev->pos();
        update();
    }
}

void GraphCanvas::mouseReleaseEvent(QMouseEvent*) {
    draggingVertex = -1;
}


