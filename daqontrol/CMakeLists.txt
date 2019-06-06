add_executable(daqontrol
    src/daqontrol.cc
    include/daq_control.h              src/daq_control.cc
    )

target_include_directories(daqontrol PUBLIC include)
target_link_libraries(daqontrol PUBLIC clb)
target_link_libraries(daqontrol PUBLIC util)
target_link_libraries(daqontrol PUBLIC Boost::thread)
target_link_libraries(daqontrol PUBLIC Boost::program_options)
target_link_libraries(daqontrol PUBLIC Boost::system)
