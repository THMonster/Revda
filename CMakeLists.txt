cmake_minimum_required(VERSION 3.6)
# SET(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Set install prefix.")
project(revda)
set(PROJECT_VERSION "5.7.0")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(ECM 1.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ECMUninstallTarget)

if (NOT NODMLIVE)
		find_program(MPV_PATH NAMES mpv REQUIRED)
		IF(${MPV_PATH} STREQUAL MPV_PATH-NOTFOUND)
				message(FATAL_ERROR "mpv not found!!!")
		ENDIF()
endif()

include(FeatureSummary)
find_package(Cargo REQUIRED)
find_package(Rust REQUIRED)

add_custom_command(
		OUTPUT "src-tauri/target/release/revda"
		COMMAND yarn
		COMMAND yarn build
		COMMAND yarn tauri build
		DEPENDS src/main.ts
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		)

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if(CMAKE_BUILD_TYPE_UPPER STREQUAL DEBUG)
		set(RUST_TARGET_DIR target/debug/)
		set(RUST_BUILD_FLAG)
else()
		set(RUST_TARGET_DIR target/release/)
		set(RUST_BUILD_FLAG --release)
endif()

if (NOT NODMLIVE)
		SET(DML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dmlive")
		if (MSVC)
				SET(DML_BIN "${DML_DIR}/${RUST_TARGET_DIR}/dmlive")
		else()
				SET(DML_BIN "${DML_DIR}/${RUST_TARGET_DIR}/dmlive")
		endif()

		add_custom_command(
				OUTPUT "${DML_BIN}"
				COMMAND ${Cargo_EXECUTABLE} build ${RUST_BUILD_FLAG}
				DEPENDS dmlive/src/main.rs
				WORKING_DIRECTORY "${DML_DIR}"
				)
		add_custom_target(dmlive-bin ALL DEPENDS "${DML_BIN}")
endif()

add_custom_target(revda-bin ALL DEPENDS "src-tauri/target/release/revda")

if (NOT NODMLIVE)
		install(PROGRAMS "${DML_BIN}" DESTINATION bin)
endif()
install(PROGRAMS "src-tauri/target/release/revda" DESTINATION bin)

install(FILES "icon/revda.svg"
		DESTINATION "share/icons/hicolor/scalable/apps")
install(FILES "misc/revda.desktop"
		DESTINATION "share/applications")
install(FILES "misc/dmlive-mime.desktop"
		DESTINATION "share/applications")
