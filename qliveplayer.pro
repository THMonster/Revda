CONFIG -= app_bundle
QT += widgets

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig debug
PKGCONFIG += mpv

HEADERS = \
    mpvwidget.h \
    mainwindow.h \
    danmakurecorder.h \
    clirecorder.h
SOURCES = main.cpp \
    mpvwidget.cpp \
    mainwindow.cpp \
    danmakurecorder.cpp \
    clirecorder.cpp

RESOURCES += \
    resource.qrc
