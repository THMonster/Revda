cmake_minimum_required(VERSION 3.0)
# SET(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Set install prefix.")
project(QLivePlayer)
set(PROJECT_VERSION "3.11.1")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_program(YKDL_PATH NAMES ykdl REQUIRED)
IF(${YKDL_PATH} STREQUAL YKDL_PATH-NOTFOUND)
	message(FATAL_ERROR "ykdl not found!!!")
ENDIF()
find_program(MPV_PATH NAMES mpv REQUIRED)
IF(${MPV_PATH} STREQUAL MPV_PATH-NOTFOUND)
	message(FATAL_ERROR "mpv not found!!!")
ENDIF()
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets Quick QuickControls2 QuickWidgets )


add_subdirectory(src)
