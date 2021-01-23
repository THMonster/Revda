set(qlp_file_SRCS
    main.cpp
    mainwindow.cpp
    room_model.cpp
    sites.cpp
    datasource.cpp
    qlpconfig.cpp
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

file(GLOB qlprecorder tools/qlprecorder)
install(PROGRAMS ${qlprecorder} DESTINATION bin)
install(TARGETS qliveplayer DESTINATION bin)
add_subdirectory(qlphelper)
add_subdirectory(qlpjs)
