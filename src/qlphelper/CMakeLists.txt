set(qlphelper_file_SRCS
    qlphelper.cpp
    danmakulauncher.cpp
)

add_executable(qlphelper ${qlphelper_file_SRCS})
target_link_libraries(qlphelper
  Qt5::Core
)

install(TARGETS qlphelper DESTINATION bin)
