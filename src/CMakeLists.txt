set(qlp_file_SRCS
    main.cpp
    mainwindow.cpp
    room_model.cpp
    sites.cpp
    datasource.cpp
    qlpconfig.cpp
    resource.qrc
)

#qt5_add_resources(qlp_file_SRCS resource.qrc)
add_executable(qliveplayer ${qlp_file_SRCS})
target_compile_definitions(qliveplayer PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(qliveplayer
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Qt6::Quick
  Qt6::Qml
  Qt6::QuickWidgets
)



file(GLOB qlprecorder tools/qlprecorder)
install(PROGRAMS ${qlprecorder} DESTINATION bin)
install(TARGETS qliveplayer DESTINATION bin)
add_subdirectory(qlphelper)
add_subdirectory(qlpjs)
