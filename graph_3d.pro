QT       += core gui

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = a.out
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
    linear_map_2d.cpp \
    main.cpp \
    window.cpp \
    glwidget.cpp \
    surface.cpp \
    thread_tools.cpp \
    geometry_data.cpp \
    sparse_tools.cpp \
    basic_matrix_tools.cpp

HEADERS += \
    linear_map_2d.h \
    window.h \
    glwidget.h \
    surface.h \
    thread_tools.h \
    global_defines.h \
    geometry_data.h \
    sparse_tools.h \
    basic_matrix_tools.h

CONFIG += c++1z \
          optimize_full
QMAKE_CXXFLAGS -= -O2
#QMAKE_CXX = clang++
#QMAKE_LINK = clang++
QMAKE_CXXFLAGS += -g

QMAKE_CXXFLAGS += -ffast-math -Ofast -funroll-all-loops -Wextra -pedantic -Wall -Wcast-qual -march=native -lm
LIBS += -pthread

QT += opengl
QT += gui
