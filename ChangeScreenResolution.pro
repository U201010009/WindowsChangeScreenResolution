#-------------------------------------------------
#
# Project created by QtCreator 2017-09-12T09:53:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChangeScreenResolution
TEMPLATE = app

SOURCES += main.cpp\
        dialog.cpp

HEADERS  += dialog.h

LIBS += User32.lib

FORMS    += dialog.ui
