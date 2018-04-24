CONFIG -= app_bundle
QT += core gui widgets

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig debug
PKGCONFIG += mpv

HEADERS = \
    mainwindow.h \
    danmakurecorder.h \
    danmakulauncher.h \
    cliclient.h \
    mpvemwidget.h \
    danmakuplayer.h \
    danmakuglwidget.h
SOURCES = main.cpp \
    mainwindow.cpp \
    danmakurecorder.cpp \
    danmakulauncher.cpp \
    cliclient.cpp \
    mpvemwidget.cpp \
    danmakuplayer.cpp \
    danmakuglwidget.cpp

RESOURCES += \
    resource.qrc
