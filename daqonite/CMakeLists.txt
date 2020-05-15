add_executable(
  daqonite
  src/daqonite.cc
  include/bbb_hit_receiver.h
  src/bbb_hit_receiver.cc
  include/daqonite_publisher.h
  src/daqonite_publisher.cc
  include/data_run_serialiser.h
  src/data_run_serialiser.cc
  include/basic_hit_receiver.h
  src/basic_hit_receiver.cc
  include/clb_hit_receiver.h
  src/clb_hit_receiver.cc
  include/daq_handler.h
  src/daq_handler.cc
  include/data_run.h
  src/data_run.cc
  include/daq_logging.h
  include/spill_schedule.h
  src/spill_schedule.cc
  include/merge_sorter.h
  src/merge_sorter.cc
  include/run_file.h
  src/run_file.cc
  include/spill_schedulers.h
  src/spill_schedulers.cc
  include/trigger_predictor.h
  src/trigger_predictor.cc)

target_include_directories(daqonite PUBLIC include)

target_link_libraries(daqonite PUBLIC clb)
target_link_libraries(daqonite PUBLIC bbb)
target_link_libraries(daqonite PUBLIC util)
target_link_libraries(daqonite PUBLIC spill_scheduling)
target_link_libraries(daqonite PUBLIC nngpp)
target_link_libraries(daqonite PUBLIC ${ROOT_LIBRARIES})
target_link_libraries(daqonite PUBLIC Boost::program_options)
target_link_libraries(daqonite PUBLIC Boost::chrono)
target_link_libraries(daqonite PUBLIC Boost::system)
target_link_libraries(daqonite PUBLIC Boost::thread)
