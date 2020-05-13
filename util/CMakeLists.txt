add_library(util 
    include/util/control_msg.h                  src/control_msg.cc
    include/util/chips_config.h                 src/chips_config.cc
    include/util/elastic_interface.h            src/elastic_interface.cc
    include/util/logging.h                      src/logging.cc
    include/util/async_runnable.h
    include/util/async_component.h              src/async_component.cc
    include/util/async_component_group.h        src/async_component_group.cc
    include/util/publisher.h
    include/util/command_receiver.h             src/command_receiver.cc
    include/util/timestamp.h                    src/timestamp.cc
    include/util/signal_receiver.h              src/signal_receiver.cc
    include/util/daq_config.h                   src/daq_config.cc
    include/util/bus_publisher.h                src/bus_publisher.cc
    include/util/singleton_process.h
    )
    
target_include_directories(util PUBLIC include)
target_include_directories(util PRIVATE include/util)

target_link_libraries(util PUBLIC ${ELASTICLIENT_LIBRARIES})
target_include_directories(util PUBLIC ${ELASTICLIENT_INCLUDE_DIRS})

target_link_libraries(util PUBLIC fmt)
target_link_libraries(util PUBLIC Boost::thread Boost::date_time)
target_link_libraries(util PUBLIC nngpp)
