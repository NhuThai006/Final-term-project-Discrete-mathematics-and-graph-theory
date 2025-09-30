#pragma once

#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include "GraphCanvas.h"
#include "Algorithms.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onAddVertex();
    void onAddEdge();
    void onMoveVertex();
    void onErase();
    void onClear();
    void onComputeEuler();
    void onComputePostman();
    void onExportImage();
    void onExportPdf();
    void onAttachFiles();
    void onImportMapBackground();
    void onClearMapBackground();
    void onExportMatrix();
    void onShowSummary();

private:
    GraphCanvas *canvas{nullptr};

    QAction *actAddVertex{nullptr};
    QAction *actAddEdge{nullptr};
    QAction *actMoveVertex{nullptr};
    QAction *actEraser{nullptr};
    QAction *actClear{nullptr};
    QAction *actEuler{nullptr};
    QAction *actPostman{nullptr};
    QAction *actExportImg{nullptr};
    QAction *actExportPdf{nullptr};
    QAction *actAttach{nullptr};
    QAction *actImportMap{nullptr};
    QAction *actClearMap{nullptr};
    QAction *actExportMatrix{nullptr};
    QPushButton *btnShowSummary{nullptr};

    void setupUi();
};


