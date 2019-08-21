add_library(relay 
    include/relay/relay.h
    include/relay/danout_relay.h             src/danout_relay.cc
    include/relay/mc_relay.h                 src/mc_relay.cc
    include/relay/ec_relay.h                 src/ec_relay.cc
    )

add_executable(power 
    src/power.cc
    include/relay/relay.h
    include/relay/danout_relay.h             src/danout_relay.cc
    include/relay/mc_relay.h                 src/mc_relay.cc
    include/relay/ec_relay.h                 src/ec_relay.cc
    )
    
target_include_directories(relay PUBLIC include)
target_include_directories(relay PRIVATE include/relay)

target_link_libraries(relay PUBLIC util)
target_link_libraries(power PUBLIC relay) 