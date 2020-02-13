set(qlphelper_file_SRCS
    qlphelper.cpp
    danmakulauncher.cpp
    fudujikiller.cpp
)

add_executable(qlphelper ${qlphelper_file_SRCS})
target_link_libraries(qlphelper
  Qt5::Core
)

target_compile_definitions(qlphelper PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

file(GLOB dmcpyz dmc.pyz)
install(FILES ${dmcpyz} DESTINATION share/QLivePlayer)

install(TARGETS qlphelper DESTINATION bin)
