add_library(spill_scheduling 
    include/spill_scheduling/spill.h
    include/spill_scheduling/spill_data_slot.h
    include/spill_scheduling/tdu_signal.h
    include/spill_scheduling/tdu_signal_type.h           src/tdu_signal_type.cc
    include/spill_scheduling/basic_spill_scheduler.h     src/basic_spill_scheduler.cc
    include/spill_scheduling/infinite_spill_scheduler.h  src/infinite_spill_scheduler.cc
    include/spill_scheduling/periodic_spill_scheduler.h  src/periodic_spill_scheduler.cc
    include/spill_scheduling/tdu_spill_scheduler.h       src/tdu_spill_scheduler.cc
    include/spill_scheduling/trigger_predictor.h         src/trigger_predictor.cc
    include/spill_scheduling/xml_rpc_spill_method.h      src/xml_rpc_spill_method.cc
    )
    
target_include_directories(spill_scheduling PUBLIC include)
target_include_directories(spill_scheduling PRIVATE include/spill_scheduling)

target_link_libraries(spill_scheduling PUBLIC util)
target_link_libraries(spill_scheduling PUBLIC XmlRpc)
