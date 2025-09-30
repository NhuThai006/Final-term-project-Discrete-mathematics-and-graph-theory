#include "MainWindow.h"
#include "ChinesePostman.h"
#include <QToolBar>
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QImageReader>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
}

void MainWindow::setupUi() {
    canvas = new GraphCanvas(this);

    // Create banner area (title + students) above the canvas
    auto central = new QWidget(this);
    auto rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto banner = new QWidget(central);
    auto bannerLayout = new QVBoxLayout(banner);
    bannerLayout->setContentsMargins(0, 8, 0, 8);
    bannerLayout->setSpacing(2);

    auto lblTitle = new QLabel("EULERIAN CIRCUITS TO ASSIST POLICE IN STREET INSPECTION IN HO CHI MINH CITY", banner);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-weight:700; font-size:25px; color:#0078FF;");

    // Lecturer line (centered)
    auto lblLecturer = new QLabel("<b>Lecturer name:</b> Assoc. Prof. Hoang Van Dung", banner);
    lblLecturer->setAlignment(Qt::AlignCenter);
    lblLecturer->setStyleSheet("font-size:16px; color:#000000; margin-top:6px; margin-bottom:24px;");

    // Students title
    auto lblStudentsTitle = new QLabel("<b>students:</b>", banner);
    lblStudentsTitle->setAlignment(Qt::AlignLeft);
    lblStudentsTitle->setStyleSheet("font-size:16px; color:#000000; margin-top:12px; margin-bottom:6px; margin-left:auto; margin-right:auto;");

    // Members table (centered) similar to screenshot
    QString tableHtml =
        "<table border='1' cellspacing='0' cellpadding='8' style='border-collapse:collapse; font-size:14px; margin-left:auto; margin-right:auto;'>"
        "<tr>"
        "<td style='min-width:110px; text-align:center'>24110117</td>"
        "<td style='min-width:320px; text-align:center'>Thai Ngoc Tam Nhu (leader)</td>"
        "</tr>"
        "<tr>"
        "<td style='text-align:center'>24110148</td>"
        "<td style='text-align:center'>Duong Duy Vinh</td>"
        "</tr>"
        "</table>";
    auto lblTable = new QLabel(tableHtml, banner);
    lblTable->setAlignment(Qt::AlignCenter);
    lblTable->setStyleSheet("color:#000000;");

    bannerLayout->addWidget(lblTitle);
    bannerLayout->addWidget(lblLecturer);
    bannerLayout->addStretch();
    bannerLayout->addWidget(lblStudentsTitle);
    bannerLayout->addWidget(lblTable);

    rootLayout->addWidget(banner);
    rootLayout->addWidget(canvas, 1);
    setCentralWidget(central);
    connect(canvas, &GraphCanvas::statusMessage, this, [this](const QString &m){ statusBar()->showMessage(m, 3000); });

    auto tb = addToolBar("Tools");
    actAttach = tb->addAction("Attach files", this, &MainWindow::onAttachFiles);
    tb->addSeparator();
    actAddVertex = tb->addAction("Add Vertex", this, &MainWindow::onAddVertex);
    actMoveVertex = tb->addAction("Move Vertex", this, &MainWindow::onMoveVertex);
    actAddEdge = tb->addAction("Add Edge", this, &MainWindow::onAddEdge);
    actEraser = tb->addAction("Eraser Edge/Vertex", this, &MainWindow::onErase);
    tb->addSeparator();
    actClear = tb->addAction("Clear All", this, &MainWindow::onClear);
    tb->addSeparator();
    actEuler = tb->addAction("Euler", this, &MainWindow::onComputeEuler);
    actPostman = tb->addAction("Postman", this, &MainWindow::onComputePostman);
    tb->addSeparator();
    actExportImg = tb->addAction("Export Image", this, &MainWindow::onExportImage);
    actExportPdf = tb->addAction("Export PDF", this, &MainWindow::onExportPdf);
    actImportMap = tb->addAction("Import Map Background", this, &MainWindow::onImportMapBackground);
    actClearMap = tb->addAction("Clear Map Background", this, &MainWindow::onClearMapBackground);
    actExportMatrix = tb->addAction("Export Matrix", this, &MainWindow::onExportMatrix);
    tb->addSeparator();
    // Blue Show Summary button next to Export PDF
    btnShowSummary = new QPushButton("Show Summary", tb);
    {
        QPalette pal = btnShowSummary->palette();
        pal.setColor(QPalette::ButtonText, QColor(0,120,255));
        btnShowSummary->setPalette(pal);
    }
    tb->addWidget(btnShowSummary);
    connect(btnShowSummary, &QPushButton::clicked, this, &MainWindow::onShowSummary);

    statusBar()->showMessage("Ready");
}

void MainWindow::onShowSummary() {
    const Graph &g = canvas->model();
    const auto &verts = g.getVertices();
    const auto &edges = g.getEdges();

    if (verts.empty()) {
        QMessageBox::information(this, "Summary", "The graph is empty.");
        return;
    }

    QString text;
    text += "🖥️ Program output summary\n\n";

    // Input Graph Information with detailed description
    text += "Input Graph Information\n";
    text += QString("The examined graph comprises %1 vertices and %2 edges, representing a segment of an urban traffic network. The edges are listed as follows:\n").arg(verts.size()).arg(edges.size());
    for (const auto &e : edges) {
        if (e.u >= 0 && e.u < verts.size() && e.v >= 0 && e.v < verts.size())
            text += QString("+ %1-%2: %3\n").arg(verts[e.u].name).arg(verts[e.v].name).arg(e.id + 1);
        else
            text += QString("+ [invalid edge: %1-%2] %3\n").arg(e.u).arg(e.v).arg(e.id + 1);
    }

    // Vertex Degrees
    text += "\nVertex Degrees\n";
    QVector<int> deg(verts.size(), 0);
    for (const auto &e : edges) { 
        if (e.u >= 0 && static_cast<size_t>(e.u) < deg.size()) deg[e.u]++; 
        if (e.v >= 0 && static_cast<size_t>(e.v) < deg.size()) deg[e.v]++; 
    }
    for (size_t i = 0; i < deg.size(); ++i) {
        text += QString("+ %1 = %2\n").arg(verts[i].name).arg(deg[i]);
    }

    // Eulerian Circuit Analysis
    text += "\nEulerian Circuit Analysis\n";
    size_t oddCount = 0;
    std::vector<int> odd;
    odd.reserve(deg.size());
    for (size_t i = 0; i < deg.size(); ++i) {
        if (deg[i] % 2 == 1) {
            ++oddCount;
            odd.push_back(static_cast<int>(i));
        }
    }

    if (edges.empty()) {
        text += "The graph is empty (trivially Eulerian).\n";
    } else if (oddCount == 0) {
        text += "The graph possesses an Eulerian circuit, as all vertices have even degrees.\n";
    } else if (oddCount == 2) {
        text += "The graph possesses an Eulerian path, with exactly two vertices of odd degree.\n";
        text += "Odd degree vertices: ";
        for (size_t i = 0; i < odd.size(); ++i) {
            text += verts[odd[i]].name;
            if (i + 1 < odd.size()) text += ", ";
        }
        text += "\n";
    } else {
        text += QString("The graph does not possess an Eulerian circuit, as %1 vertices ").arg(odd.size());
        text += "exhibit odd degrees. According to Euler's theorem, an Eulerian circuit is feasible only when all vertices have an even degree. ";
        text += "The presence of multiple odd-degree vertices precludes both an Eulerian circuit and an Eulerian trail in the current graph configuration.\n\n";
        text += "Odd degree vertices: ";
        for (size_t i = 0; i < odd.size(); ++i) {
            text += verts[odd[i]].name;
            if (i + 1 < odd.size()) text += ", ";
        }
        text += "\n";
    }

    // Route Analysis
    auto eulerRes = Algorithms::findEulerTourHierholzer(g);
    if (eulerRes && !eulerRes->edgeOrder.empty()) {
        text += "\nEulerian Path Solution\n";
        QStringList eids;
        QStringList vseq;
        int curr = -1;
        // Tìm startVertex an toàn
        const Edge &e0 = edges[eulerRes->edgeOrder[0]];
        int startVertex = (e0.u >= 0 && e0.u < verts.size()) ? e0.u : 0;
        curr = startVertex;
        vseq << verts[curr].name;
        for (int eid : eulerRes->edgeOrder) {
            if (eid < 0 || eid >= edges.size()) continue;
            const Edge &E = edges[eid];
            int next = (E.u == curr) ? E.v : E.u;
            if (next < 0 || next >= verts.size()) break;
            vseq << verts[next].name;
            curr = next;
            eids << QString::number(eid + 1);
        }
        text += eulerRes->isCycle ? "Eulerian cycle found:\n" : "Eulerian path found:\n";
        text += "Vertex order: " + vseq.join(" -> ") + "\n";
        text += "Edge traversal: " + eids.join(", ") + "\n";
    } else {
        text += "\nChinese Postman (approx):\n";
        auto post = ChinesePostmanOptimal::solve(g);
        if (!post.edgeOrder.empty()) {
            QStringList eids;
            QStringList vseq;
            int curr = -1;
            // Tìm startVertex an toàn
            int firstEid = post.edgeOrder[0];
            const auto& edges = g.getEdges();
            const auto& verts = g.getVertices();
            int startVertex = 0;
            if (firstEid >= 0 && static_cast<size_t>(firstEid) < edges.size()) {
                const Edge &e0 = edges[firstEid];
                startVertex = (e0.u >= 0 && static_cast<size_t>(e0.u) < verts.size()) ? e0.u : 0;
            }
            curr = startVertex;
            vseq << verts[curr].name;
            for (int eid : post.edgeOrder) {
                if (eid < 0) continue;
                if (eid < edges.size()) {
                    const Edge &E = edges[eid];
                    int next = (E.u == curr) ? E.v : E.u;
                    if (next < 0 || static_cast<size_t>(next) >= verts.size()) break;
                    vseq << verts[next].name;
                    curr = next;
                    eids << QString::number(eid + 1);
                } else {
                    // duplicate edge
                    eids << QString("%1 (duplicate edge)").arg(eid + 1);
                }
            }
            text += vseq.join(" -> ") + "\n";
            text += "Edge order: " + eids.join(", ") + "\n";
        } else {
            text += "No valid route found.\n";
        }
    }

    // Application Context
    text += "\nApplication Context\n";
    text += "This system leverages graph theory foundations—specifically Eulerian circuit concepts and the ";
    text += "Chinese Postman Problem—to analyze and optimize urban traffic patrol routes by modeling ";
    text += "real-world city roads as graphs. The tool assists police officers in systematically inspecting all ";
    text += "street segments while minimizing redundant travel. Potential benefits include increased ";
    text += "operational efficiency, reduced travel distances and costs, and improved traffic security and ";
    text += "safety, with direct applicability in cities such as Ho Chi Minh City.\n";

    QMessageBox::information(this, "Summary", text);
}

void MainWindow::onAddVertex() { canvas->setMode(GraphCanvas::AddVertex); }
void MainWindow::onAddEdge() { canvas->setMode(GraphCanvas::AddEdge); }
void MainWindow::onMoveVertex() { canvas->setMode(GraphCanvas::MoveVertex); }
void MainWindow::onErase() { canvas->setMode(GraphCanvas::Eraser); }

void MainWindow::onClear() {
    canvas->model().clear();
    canvas->clearRoute();
    statusBar()->showMessage("Cleared graph", 2000);
    canvas->update();
}

void MainWindow::onComputeEuler() {
    auto res = Algorithms::findEulerTourHierholzer(canvas->model());
    if (!res) {
        QMessageBox::information(this, "Euler", "No Euler path/cycle exists (graph not Eulerian or semi-Eulerian). Try Postman.");
        return;
    }
    canvas->setRoute(res->edgeOrder);
    statusBar()->showMessage(res->isCycle ? "Euler cycle found" : "Euler path found", 3000);
}

void MainWindow::onComputePostman() {
    auto res = ChinesePostmanOptimal::solve(canvas->model());
    if (res.edgeOrder.empty()) {
        QMessageBox::warning(this, "Postman", "Failed to compute route.");
        return;
    }
    canvas->setRoute(res.edgeOrder);
    statusBar()->showMessage("Postman route (optimal) computed", 3000);
}

void MainWindow::onExportImage() {
    QString file = QFileDialog::getSaveFileName(this, "Export Image", {}, "PNG Image (*.png)");
    if (file.isEmpty()) return;
    QImage img(canvas->size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    canvas->render(&p);
    img.save(file);
    statusBar()->showMessage("Exported image", 2000);
}

void MainWindow::onExportPdf() {
    QString file = QFileDialog::getSaveFileName(this, "Export PDF", {}, "PDF (*.pdf)");
    if (file.isEmpty()) return;
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(file);
    printer.setFullPage(true);
    QPainter painter(&printer);
    canvas->render(&painter);
    statusBar()->showMessage("Exported PDF", 2000);
}

void MainWindow::onAttachFiles() {
    // Chọn file txt chứa ma trận kề vuông, phần tử cách nhau bởi khoảng trắng hoặc dấu phẩy
    QString file = QFileDialog::getOpenFileName(this, "Open adjacency matrix", {}, "Text files (*.txt);;All files (*.*)");
    if (file.isEmpty()) return;

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Attach files", "Không thể mở file.");
        return;
    }

    QVector<QVector<int>> mat;
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        // thay dấu phẩy bằng khoảng trắng
        line.replace(',', ' ');
        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        QVector<int> row;
        for (const QString &p : parts) row.push_back(p.toInt());
        if (!row.isEmpty()) mat.push_back(row);
    }
    if (mat.isEmpty() || mat.size() != mat[0].size()) {
        QMessageBox::warning(this, "Attach files", "Ma trận phải là ma trận vuông.");
        return;
    }

    // Dựng đồ thị mới từ ma trận
    Graph &g = canvas->model();
    g.clear();

    int n = mat.size();
    // Đặt đỉnh theo vòng tròn để dễ nhìn
    QSize sz = canvas->size();
    QPointF center(sz.width() / 2.0, sz.height() / 2.0);
    double radius = qMin(sz.width(), sz.height()) * 0.35;
    for (int i = 0; i < n; ++i) {
        double ang = (2 * M_PI * i) / n - M_PI / 2; // bắt đầu từ trên cùng
        QPointF pos(center.x() + radius * cos(ang), center.y() + radius * sin(ang));
        g.addVertex(pos);
    }
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (mat[i][j] != 0) g.addEdge(i, j);
        }
    }

    canvas->clearRoute();
    canvas->update();
    statusBar()->showMessage("Đã nhập đồ thị từ ma trận kề", 3000);
}

void MainWindow::onImportMapBackground() {
    QString file = QFileDialog::getOpenFileName(this, "Import Map Background", {}, "Images (*.png *.jpg *.jpeg *.bmp)");
    if (file.isEmpty()) return;
    QImageReader reader(file);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    if (img.isNull()) {
        QMessageBox::warning(this, "Import Map Background", "Không đọc được ảnh.");
        return;
    }
    canvas->setBackgroundImage(img);
    statusBar()->showMessage("Đã nhập ảnh nền", 2000);
}

void MainWindow::onClearMapBackground() {
    canvas->clearBackgroundImage();
    statusBar()->showMessage("Đã xóa ảnh nền", 2000);
}

void MainWindow::onExportMatrix() {
    const Graph &g = canvas->model();
    const auto &verts = g.getVertices();
    const auto &edges = g.getEdges();
    int n = static_cast<int>(verts.size());
    if (n == 0) {
        QMessageBox::information(this, "Export Matrix", "Đồ thị trống.");
        return;
    }
    QVector<QVector<int>> mat(n, QVector<int>(n, 0));
    for (const auto &e : edges) {
        if (e.u >= 0 && e.v >= 0 && e.u < n && e.v < n) {
            mat[e.u][e.v] = 1;
            mat[e.v][e.u] = 1;
        }
    }

    QString file = QFileDialog::getSaveFileName(this, "Save adjacency matrix", {}, "Text files (*.txt)");
    if (file.isEmpty()) return;
    QFile f(file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Matrix", "Không thể ghi file.");
        return;
    }
    QTextStream out(&f);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            out << mat[i][j];
            if (j + 1 < n) out << ' ';
        }
        out << '\n';
    }
    statusBar()->showMessage("Đã xuất ma trận kề", 3000);
}













