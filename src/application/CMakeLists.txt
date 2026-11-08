get_filename_component(PARENT_DIR ../../.. ABSOLUTE)

message(PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR})
message(CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})

# Fetch dependency on CLI11 header command line options parser.
include(FetchContent)


# 2. CLI11 is a header only library.
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG        v2.7.2
)
# CRITICAL FIX A: Inform the build system that SoapySDR is active inside this source tree.
# This prevents it from escaping out to /opt/homebrew configurations.
FetchContent_GetProperties(cli11)
if(NOT cli11_POPULATED)
    FetchContent_Populate(cli11)
    set(SOAPY_SDR_IN_TREE_SOURCE_DIR "${cli11_SOURCE_DIR}" CACHE INTERNAL "")
    add_subdirectory(${cli11_SOURCE_DIR} ${cli11_BINARY_DIR})
endif()
include_directories(${cli11_SOURCE_DIR}/include)



# 1. Define the executable target framework
add_executable(application main.cpp
        MockProcessor.cpp
        MockProcessor.h
        ApplicationProcessor.cpp
        ApplicationProcessor.h
        Controller.cpp
        Controller.h)

set_target_properties(application PROPERTIES
        POSITION_INDEPENDENT_CODE 1
)


if (APPLE)
    target_compile_definitions(application PRIVATE VKFFT_BACKEND=0)
    # Append the nested include path for glslang
    target_include_directories(application PRIVATE "/opt/homebrew/include")
    target_include_directories(application PRIVATE "/opt/homebrew/include/glslang/Include")
    target_link_directories(application PRIVATE /opt/homebrew/lib)
else()

    target_include_directories(client_static PRIVATE /usr/include)
    target_include_directories(client_static PRIVATE /usr/include/glslang/Include/)

endif ()
# Core link layer: Always link against your static drawing engine target
target_link_libraries(application PRIVATE client_static)
