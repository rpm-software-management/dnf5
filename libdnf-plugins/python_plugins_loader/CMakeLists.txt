if(NOT WITH_PYTHON3)
    return()
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIRS})

add_library(python_plugins_loader MODULE python_plugins_loader.cpp)

target_link_libraries(python_plugins_loader PUBLIC libdnf libdnf-cli)
target_link_libraries(python_plugins_loader PUBLIC ${PYTHON_LIBRARY})
