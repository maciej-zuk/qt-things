#-------------------------------------------------
#
# Project created by QtCreator 2013-05-28T21:16:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vr-demo
TEMPLATE = app


SOURCES += main.cpp videorecorderdemo.cpp ../videorecorder.cpp

HEADERS  += videorecorderdemo.h ../videorecorder.h

FORMS    += videorecorderdemo.ui

LIBS += -lavcodec -lavutil -lavformat -lswscale
