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
    trap.cpp \
    window.cpp \
    glwidget.cpp \
    surface.cpp \
    threading.cpp \
    geometry_data.cpp \
    sparse_func.cpp \
    matrix_func.cpp

HEADERS += \
    linear_map_2d.h \
    vector3d_d.h \
    vector4d_d.h \
    window.h \
    glwidget.h \
    surface.h \
    threading.h \
    global_defines.h \
    geometry_data.h \
    sparse_func.h \
    matrix_func.h

CONFIG += c++1z \
          optimize_full
#QMAKE_CXXFLAGS -= -O2
#QMAKE_CXX = clang++
#QMAKE_LINK = clang++
QMAKE_CXXFLAGS += -g
#QMAKE_CXXFLAGS += -fsanitize=address
#QMAKE_LFLAGS += -fsanitize=address
QMAKE_CXXFLAGS += -ffast-math -Ofast -funroll-all-loops -march=native
QMAKE_CXXFLAGS += -Wextra -pedantic -Wall -Wcast-qual \
    -pedantic-errors -Wfloat-equal -Wpointer-arith -Wformat-security \
    -Wmissing-format-attribute -Wformat=1 -Wwrite-strings -Wcast-align\
    -Wno-long-long -Woverloaded-virtual -Wnon-virtual-dtor -Wcast-qual\
    -Wno-suggest-attribute=format
LIBS += -pthread

QT += opengl
QT += gui
