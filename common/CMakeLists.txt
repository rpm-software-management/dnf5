file(GLOB_RECURSE COMMON_SOURCES *.cpp *.c)

include_directories(.)

add_library(common_obj OBJECT ${COMMON_SOURCES})
set_property(TARGET common_obj PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(common_obj PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden)

# required by clang
target_link_libraries(common_obj PUBLIC stdc++)
