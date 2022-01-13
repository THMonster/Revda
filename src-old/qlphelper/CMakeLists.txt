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
  "${RUST_LIB}"
  ${CMAKE_DL_LIBS}
)

install(TARGETS qlphelper DESTINATION bin)
