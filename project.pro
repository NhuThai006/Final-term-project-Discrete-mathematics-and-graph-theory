QT += core gui widgets printsupport

CONFIG += console
CONFIG -= app_bundle
QMAKE_CXXFLAGS += -std=c++17

TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/Algorithms.cpp \
    src/Graph.cpp \
    src/GraphCanvas.cpp \
    src/MainWindow.cpp \
    src/ChinesePostman.cpp

HEADERS += \
    src/Algorithms.h \
    src/Graph.h \
    src/GraphCanvas.h \
    src/MainWindow.h \
    src/ChinesePostman.h
