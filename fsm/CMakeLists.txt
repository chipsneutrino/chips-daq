add_executable(fsm 
    src/fsm.cc
    include/global.h                            src/global.cc

    include/ops_uplink.h                        src/ops_uplink.cc
    
    include/control_bus/events.h
    include/control_bus/fsm.h                   src/control_bus/fsm.cc
    include/control_bus/states.h                src/control_bus/states.cc
    include/control_bus/bus_master.h            src/control_bus/bus_master.cc

    include/daqonite/events.h
    include/daqonite/fsm.h                      src/daqonite/fsm.cc
    include/daqonite/states.h                   src/daqonite/states.cc
    include/daqonite/observer.h                 src/daqonite/observer.cc
    
    include/experiment/fsm.h                    src/experiment/fsm.cc
    include/experiment/states.h                 src/experiment/states.cc
    )

target_include_directories(fsm PUBLIC include)

target_link_libraries(fsm PUBLIC nngpp)
target_link_libraries(fsm PUBLIC tinyfsm)
target_link_libraries(fsm PUBLIC util)
