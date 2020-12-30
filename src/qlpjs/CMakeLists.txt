set(qlphelper_file_SRCS
    main.cpp
    jsengine.cpp
)

add_executable(qlpjs ${qlphelper_file_SRCS})
target_link_libraries(qlpjs
  Qt6::Core
  Qt6::Qml
)

target_compile_definitions(qlpjs PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

install(TARGETS qlpjs DESTINATION bin)
