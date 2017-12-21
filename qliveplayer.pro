CONFIG -= app_bundle
QT += widgets

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig debug
PKGCONFIG += mpv

HEADERS = \
    mpvwidget.h \
    mainwindow.h \
    danmakurecorder.h \
    danmakulauncher.h \
    danmakuglwidget.h \
    cliclient.h
SOURCES = main.cpp \
    mpvwidget.cpp \
    mainwindow.cpp \
    danmakurecorder.cpp \
    danmakulauncher.cpp \
    danmakuglwidget.cpp \
    cliclient.cpp

RESOURCES += \
    resource.qrc
