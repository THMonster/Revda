string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if(CMAKE_BUILD_TYPE_UPPER STREQUAL DEBUG)
    set(RUST_TARGET_DIR target/debug/)
    set(RUST_BUILD_FLAG)
else()
    set(RUST_TARGET_DIR target/release/)
    set(RUST_BUILD_FLAG --release)
endif()

cmake_policy(SET CMP0046 NEW)
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0005 NEW)

SET(RUST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dmlive")
if (MSVC)
  SET(RUST_BIN "${RUST_DIR}/${RUST_TARGET_DIR}/dmlive")
else()
  SET(RUST_BIN "${RUST_DIR}/${RUST_TARGET_DIR}/dmlive")
endif()

add_custom_command(
  OUTPUT "${RUST_BIN}"
  COMMAND ${Cargo_EXECUTABLE} build ${RUST_BUILD_FLAG}
  DEPENDS dmlive/src/main.rs
  WORKING_DIRECTORY "${RUST_DIR}"
)
add_custom_target(dmlive-bin ALL DEPENDS "${RUST_BIN}")

set(qlp_file_SRCS
    main.cpp
    mainwindow.cpp
    room_model.cpp
    sites.cpp
    datasource.cpp
    qlpconfig.cpp
    launcher.cpp
    resource.qrc
)

add_executable(qliveplayer ${qlp_file_SRCS})
target_compile_definitions(qliveplayer PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(qliveplayer
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::Quick
  Qt5::Qml
  Qt5::QuickWidgets
)

install(TARGETS qliveplayer DESTINATION bin)
install(PROGRAMS "${RUST_BIN}" DESTINATION bin)
add_subdirectory(qlpjs)
