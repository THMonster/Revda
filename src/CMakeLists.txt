set(qlp_file_SRCS
    main.cpp
    mainwindow.cpp
    room_model.cpp
    sites.cpp
)

qt5_add_resources(qlp_file_SRCS resource.qrc)
add_executable(qliveplayer ${qlp_file_SRCS})
target_compile_definitions(qliveplayer PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(qliveplayer
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::QuickControls2
  Qt5::QuickWidgets
)



install(TARGETS qliveplayer DESTINATION bin)
add_subdirectory(qlphelper)
