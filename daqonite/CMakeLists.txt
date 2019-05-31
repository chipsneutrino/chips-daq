add_executable(daqonite
    src/daqonite.cc
    include/batch_scheduler.h          src/batch_scheduler.cc
    include/bbb_api.h
    include/bbb_handler.h              src/bbb_handler.cc
    include/bus_publisher.h            src/bus_publisher.cc
    include/clb_event.h
    include/clb_handler.h              src/clb_handler.cc
    include/command_receiver.h         src/command_receiver.cc
    include/daq_handler.h              src/daq_handler.cc
    include/daq_logging.h
    include/data_handler.h             src/data_handler.cc
    include/merge_sorter.h             src/merge_sorter.cc
    include/monitoring_config.h
    include/run_file.h                 src/run_file.cc
    include/signal_receiver.h          src/signal_receiver.cc
    include/spill_scheduler.h          src/spill_scheduler.cc
    include/trigger_predictor.h        src/trigger_predictor.cc
    )

target_include_directories(daqonite PUBLIC include)

target_link_libraries(daqonite PUBLIC clb)
target_link_libraries(daqonite PUBLIC util)
target_link_libraries(daqonite PUBLIC fh_message)
target_link_libraries(daqonite PUBLIC XmlRpc)
target_link_libraries(daqonite PUBLIC nngpp)
target_link_libraries(daqonite PUBLIC ${ROOT_LIBRARIES})
target_link_libraries(daqonite PUBLIC Boost::program_options)
target_link_libraries(daqonite PUBLIC Boost::chrono)
target_link_libraries(daqonite PUBLIC Boost::system)
target_link_libraries(daqonite PUBLIC Boost::thread)
