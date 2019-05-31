add_executable(fsm src/fsm.cc)
target_link_libraries(fsm PUBLIC nngpp)
target_link_libraries(fsm PUBLIC tinyfsm)
target_link_libraries(fsm PUBLIC util)