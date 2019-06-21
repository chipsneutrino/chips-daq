add_executable(daqontrol
    src/daqontrol.cc
    include/daq_control.h              src/daq_control.cc
    include/controller.h               src/controller.cc
    include/msg_processor.h            src/msg_processor.cc
    include/mcf_message.h              src/mcf_message.cc
    include/mcf_packet.h               src/mcf_packet.cc
    include/msg_writer.h               src/msg_writer.cc
    include/msg_reader.h               src/msg_reader.cc
    include/msg_types.h
    include/proc_var.h
    )

target_include_directories(daqontrol PUBLIC include)
target_link_libraries(daqontrol PUBLIC clb)
target_link_libraries(daqontrol PUBLIC util)
target_link_libraries(daqontrol PUBLIC Boost::thread)
target_link_libraries(daqontrol PUBLIC Boost::program_options)
target_link_libraries(daqontrol PUBLIC Boost::system)
