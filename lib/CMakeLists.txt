set(NNG_TESTS NO CACHE BOOL "")
set(NNG_TOOLS YES CACHE BOOL "")

add_subdirectory(nng)

#add_subdirectory(nngpp)
add_library(nngpp INTERFACE)
target_include_directories(nngpp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/nngpp/include")
target_link_libraries(nngpp INTERFACE nng)

add_subdirectory(elasticlient)

add_subdirectory(fmt)

#add_subdirectory(tinyfsm)
add_library(tinyfsm INTERFACE)
target_include_directories(tinyfsm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/tinyfsm/include")

add_subdirectory(XmlRpc)
