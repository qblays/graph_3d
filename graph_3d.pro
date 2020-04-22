QT       += core gui

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = a.out
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    window.cpp \
    glwidget.cpp \
    surface.cpp \
    thread_tools.cpp \
    geometry_data.cpp \
    sparse_tools.cpp \
    basic_matrix_tools.cpp

HEADERS += \
    window.h \
    glwidget.h \
    surface.h \
    thread_tools.h \
    global_defines.h \
    geometry_data.h \
    sparse_tools.h \
    basic_matrix_tools.h

QMAKE_CXXFLAGS += -std=c++11 -pthread --fast-math
LIBS += -pthread

QT += opengl
QT += gui
