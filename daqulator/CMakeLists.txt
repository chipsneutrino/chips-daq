add_executable(daqulator
    src/daqulator.cc
    include/packet_generator.h         src/packet_generator.cc
    )

target_include_directories(daqulator PUBLIC include)
target_link_libraries(daqulator PUBLIC clb)
target_link_libraries(daqulator PUBLIC util)
target_link_libraries(daqulator PUBLIC Boost::thread)
target_link_libraries(daqulator PUBLIC Boost::program_options)
target_link_libraries(daqulator PUBLIC Boost::system)
target_link_libraries(daqulator PUBLIC ${ROOT_LIBRARIES})
