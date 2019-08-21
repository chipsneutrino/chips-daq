add_executable(daqontrol
    src/daqontrol.cc
    include/daq_control.h              src/daq_control.cc
    include/controller.h
    include/clb_controller.h           src/clb_controller.cc
    include/bbb_controller.h           src/bbb_controller.cc
    include/daqontrol_publisher.h      src/daqontrol_publisher.cc
    )

target_include_directories(daqontrol PUBLIC include)
target_link_libraries(daqontrol PUBLIC clb)
target_link_libraries(daqontrol PUBLIC util)
target_link_libraries(daqontrol PUBLIC relay)
target_link_libraries(daqontrol PUBLIC Boost::thread)
target_link_libraries(daqontrol PUBLIC Boost::program_options)
target_link_libraries(daqontrol PUBLIC Boost::system)
target_link_libraries(daqontrol PUBLIC fh_message)
