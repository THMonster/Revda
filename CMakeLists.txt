cmake_minimum_required(VERSION 3.0)
# SET(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Set install prefix.")
project(QLivePlayer)
set(PROJECT_VERSION "4.0.1")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(ECM 1.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ECMUninstallTarget)
find_program(MPV_PATH NAMES mpv REQUIRED)
IF(${MPV_PATH} STREQUAL MPV_PATH-NOTFOUND)
	message(FATAL_ERROR "mpv not found!!!")
ENDIF()

include(FeatureSummary)
find_package(Cargo REQUIRED)
find_package(Rust REQUIRED)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
#find_package(OpenSSL REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets Network Quick Qml QuickWidgets REQUIRED)

install(FILES "icon/qliveplayer.svg"
        DESTINATION "share/icons/hicolor/scalable/apps")
install(FILES "misc/qliveplayer.desktop"
        DESTINATION "share/applications")
install(FILES "misc/qlp-mime.desktop"
        DESTINATION "share/applications")

add_subdirectory(src)
