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
   ../Binding.cpp
)

add_executable(qlphelper ${qlphelper_file_SRCS})
target_compile_definitions(qlphelper PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
add_dependencies(qlphelper rust_target)
target_link_libraries(qlphelper
  Qt5::Core
  Qt5::Network
  Threads::Threads
  OpenSSL::Crypto
  OpenSSL::SSL
  "${RUST_LIB}"
  ${CMAKE_DL_LIBS}
)

#file(GLOB dmcpyz dmc.pyz streamfinder.pyz)
install(FILES ${dmcpyz} DESTINATION share/QLivePlayer)

install(TARGETS qlphelper DESTINATION bin)
