add_library(clb
    include/clb/clb_event.h                    src/clb_event.cc
    include/clb/clb_subsys.h
    include/clb/data_structs.h
    include/clb/header_structs.h
    include/clb/msg_builder.h                  src/msg_builder.cc
    include/clb/msg_processor.h                src/msg_processor.cc
    include/clb/msg_reader.h                   src/msg_reader.cc
    include/clb/msg_types.h
    include/clb/msg_writer.h                   src/msg_writer.cc
    include/clb/proc_var.h
    include/clb/var_info.h                     src/var_info.cc
    include/clb/var_types.h
    )

target_include_directories(clb PUBLIC include)
target_include_directories(clb PRIVATE include/clb)

target_link_libraries(clb PUBLIC util)
