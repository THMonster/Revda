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
    cliclient.h \
    danmakuglwidget.h
SOURCES = main.cpp \
    mpvwidget.cpp \
    mainwindow.cpp \
    danmakurecorder.cpp \
    danmakulauncher.cpp \
    cliclient.cpp \
    danmakuglwidget.cpp

RESOURCES += \
    resource.qrc
