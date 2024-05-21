file(GLOB_RECURSE COMMON_SOURCES *.cpp *.c)

include_directories(.)

add_library(common OBJECT ${COMMON_SOURCES})
set_property(TARGET common PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(common PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden)

# required by clang
target_link_libraries(common PUBLIC stdc++)
