add_executable(daqontrol
    src/daqontrol.cc
    )

target_include_directories(daqontrol PUBLIC include)
target_link_libraries(daqontrol PUBLIC clb)
target_link_libraries(daqontrol PUBLIC util)
target_link_libraries(daqontrol PUBLIC Boost::thread)
target_link_libraries(daqontrol PUBLIC Boost::program_options)
target_link_libraries(daqontrol PUBLIC Boost::system)
