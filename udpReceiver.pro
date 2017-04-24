#-------------------------------------------------
#
# Project created by QtCreator 2016-03-15T15:10:56
#
#-------------------------------------------------

QT       += core gui
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = udpReceiver
TEMPLATE = app

INCLUDEPATH += serialPort\

SOURCES += main.cpp\
        widget.cpp \
    serialPort/qextserialbase.cpp \
    serialPort/qextserialport.cpp \
    serialPort/win_qextserialport.cpp \
    mythread.cpp

HEADERS  += widget.h \
    serialPort/qextserialbase.h \
    serialPort/qextserialport.h \
    serialPort/win_qextserialport.h \
    mythread.h

FORMS    += widget.ui

QMAKE_CXXFLAGS +=  -Wno-unused-parameter
