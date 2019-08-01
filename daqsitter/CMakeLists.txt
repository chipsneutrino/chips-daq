add_executable(daqsitter
    src/daqsitter.cc
    include/monitoring_handler.h        src/monitoring_handler.cc
    include/daqsitter_publisher.h       src/daqsitter_publisher.cc
    )

target_include_directories(daqsitter PUBLIC include)

target_link_libraries(daqsitter PUBLIC clb)
target_link_libraries(daqsitter PUBLIC util)
target_link_libraries(daqsitter PUBLIC Boost::thread)
target_link_libraries(daqsitter PUBLIC Boost::program_options)
target_link_libraries(daqsitter PUBLIC Boost::system)
target_link_libraries(daqsitter PUBLIC ${ROOT_LIBRARIES})
