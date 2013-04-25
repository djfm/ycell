#-------------------------------------------------
#
# Project created by QtCreator 2013-04-11T08:34:40
#
#-------------------------------------------------

QMAKE_CXXFLAGS += -std=c++11

QT       += core gui script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = ycell
TEMPLATE = app


SOURCES +=  main.cpp\
            mainwindow.cpp \
            sheetview.cpp \
            sheetmodel.cpp \
            csv.cpp \
            sparsetable.cpp \
    sheetviewdelegate.cpp \
    sheetcell.cpp \
    celleditor.cpp \
    refsolver.cpp \
    sheet.cpp \
    app.cpp

HEADERS  += mainwindow.h \
            sheetview.h \
            sheetmodel.h \
            csv.h \
            sparsetable.h \
    sheetviewdelegate.h \
    sheetcell.h \
    celleditor.h \
    refsolver.h \
    sheet.h \
    app.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    common.js
