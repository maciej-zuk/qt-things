#-------------------------------------------------
#
# Project created by QtCreator 2013-06-01T19:24:38
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vp-demo
TEMPLATE = app


SOURCES += main.cpp videoplayerdemo.cpp ../videoplayer.cpp

HEADERS  += videoplayerdemo.h ../videoplayer.h

FORMS    += videoplayerdemo.ui

LIBS += -lavcodec -lavformat -lavutil -lswscale
