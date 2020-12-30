set(qlphelper_file_SRCS
    main.cpp
    qlphelper.cpp
    danmakulauncher.cpp
    fudujikiller.cpp
    streamer.cpp
    mpvcontrol.cpp
    ffmpegcontrol.cpp
    bilivideo.cpp
    streamfinder.cpp
    qlpcmdparser.cpp
    ../qlpconfig.cpp
)

add_executable(qlphelper ${qlphelper_file_SRCS})
target_compile_definitions(qlphelper PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(qlphelper
  Qt6::Core
  Qt6::Network
)


file(GLOB dmcpyz dmc.pyz streamfinder.pyz)
install(FILES ${dmcpyz} DESTINATION share/QLivePlayer)

install(TARGETS qlphelper DESTINATION bin)
