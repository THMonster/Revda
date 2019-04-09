CONFIG -= app_bundle
QT += dbus widgets

QT_CONFIG -= no-pkg-config
CONFIG += release link_pkgconfig
PKGCONFIG += mpv

HEADERS = \
    mpvwidget.h \
    mainwindow.h \
    danmakurecorder.h \
    danmakulauncher.h \
    cliclient.h
SOURCES = main.cpp \
    mpvwidget.cpp \
    mainwindow.cpp \
    danmakurecorder.cpp \
    danmakulauncher.cpp \
    cliclient.cpp

RESOURCES += \
    resource.qrc
