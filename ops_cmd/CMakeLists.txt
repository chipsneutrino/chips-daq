add_executable(ops_cmd src/ops_cmd.cc)
#target_link_libraries(ops_cmd PUBLIC nng)

target_link_libraries(ops_cmd PUBLIC nngpp)

target_link_libraries(ops_cmd PUBLIC util)