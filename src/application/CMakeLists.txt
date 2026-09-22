get_filename_component(PARENT_DIR ../../.. ABSOLUTE)

message(PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR})
message(CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})

# 1. Define the executable target framework
add_executable(application main.cpp
        MockProcessor.cpp
        MockProcessor.h)

set_target_properties(application PROPERTIES
        POSITION_INDEPENDENT_CODE 1
)

# Core link layer: Always link against your static drawing engine target
target_link_libraries(application PRIVATE client_static)
