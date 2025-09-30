#pragma once

#include <QWidget>
#include <QPainterPath>
#include <QPen>
#include <QImage>
#include "Graph.h"

class GraphCanvas : public QWidget {
    Q_OBJECT
public:
    explicit GraphCanvas(QWidget *parent = nullptr);

    Graph& model() { return graph; }
    const Graph& model() const { return graph; }

    void setRoute(const std::vector<int>& edgeOrder);
    void clearRoute();
    void setBackgroundImage(const QImage &img) { backgroundImage = img; update(); }
    void clearBackgroundImage() { backgroundImage = QImage(); update(); }

    enum Mode { AddVertex, AddEdge, MoveVertex, Eraser, None };
    void setMode(Mode m) { mode = m; }

signals:
    void statusMessage(const QString &msg);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    Graph graph;
    std::vector<int> routeEdgeOrder;
    Mode mode{AddVertex};
    int pendingEdgeFrom{-1};
    int draggingVertex{-1};
    QImage backgroundImage;

    int hitTestVertex(const QPointF &p) const;
    int hitTestEdge(const QPointF &p, int *outU = nullptr, int *outV = nullptr, double threshold = 6.0) const;
};


