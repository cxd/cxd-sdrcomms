cmake_minimum_required(VERSION 3.10)
project(sdrcomms LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_SDR_DEBUG "Enable verbose PlutoSDR channel debugging" OFF)


include(FetchContent)


# 2. Fetch SoapySDR Core
FetchContent_Declare(
        soapysdr
        GIT_REPOSITORY https://github.com/pothosware/SoapySDR.git
        GIT_TAG        master
)

# CRITICAL FIX A: Inform the build system that SoapySDR is active inside this source tree.
# This prevents it from escaping out to /opt/homebrew configurations.
FetchContent_GetProperties(soapysdr)
if(NOT soapysdr_POPULATED)
    FetchContent_Populate(soapysdr)
    set(SOAPY_SDR_IN_TREE_SOURCE_DIR "${soapysdr_SOURCE_DIR}" CACHE INTERNAL "")
    add_subdirectory(${soapysdr_SOURCE_DIR} ${soapysdr_BINARY_DIR})
endif()

# CRITICAL FIX B: Tell subsequent dependencies (like SoapyRemote) to never check
# global paths for SoapySDR because we have an active CMake alias target.
set(SoapySDR_DIR "${soapysdr_BINARY_DIR}" CACHE PATH "Force local SoapySDR path" FORCE)

# Disable building apps/tests to keep your build clean and fast
set(ENABLE_APPS OFF CACHE INTERNAL "")
set(ENABLE_TESTS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(soapysdr)

# 3. Fetch SoapyPluto Plugin
FetchContent_Declare(
        soapyremote
        GIT_REPOSITORY https://github.com/pothosware/SoapyRemote.git
        GIT_TAG        master
)


# Tell the remote build explicitly to look AT the fetched binary directory first,
# and tell find_package to ignore global paths (/opt/homebrew or /usr) completely.
set(SoapySDR_DIR "${soapysdr_BINARY_DIR}" CACHE PATH "" FORCE)
set(CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY ON)
set(CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY ON)

FetchContent_MakeAvailable(soapyremote)


add_library(client_static STATIC include/client.h
        client.cpp
        IQBlock.cpp
        include/IQBlock.h
        ReceiveQueue.cpp
        include/ReceiveQueue.h
        include/constants.h
        SampleProcessor.cpp
        include/SampleProcessor.h
        SampleProcessorImpl.cpp
        include/SampleProcessorImpl.h
        SampleStreamerProcessor.cpp
        include/SampleStreamerProcessor.h
        NetIQBlock.cpp
        include/NetIQBlock.h)

# 2. If the option is turned ON, inject a preprocessor macro into the C++ compiler
if(ENABLE_SDR_DEBUG)
    target_compile_definitions(client_static PRIVATE CL_DEBUG_PLUTO=1)
    message(STATUS ">>> PlutoSDR Conditional Debugging: ENABLED <<<")
endif()


# 1. Manually pull the source directory variable from the fetched SoapySDR configuration
FetchContent_GetProperties(soapysdr)
if(soapysdr_POPULATED)
    # 2. Force export the raw header folder path downstream.
    # Use PUBLIC so it applies to both this library AND your main application target.
    target_include_directories(client_static PUBLIC
            "${soapysdr_SOURCE_DIR}/include"
    )
endif()

target_link_libraries(client_static PRIVATE SoapySDR)

# 2. Tell CMake to build the remoteSupport plugin whenever my_radio_app is built
add_dependencies(client_static remoteSupport)

# 3. Automatically inject runtime search paths for your IDE/Debugger
# This embeds the built SoapySDR library directory into your executable path info
set_target_properties(client_static PROPERTIES
        BUILD_RPATH "${soapysdr_BINARY_DIR}/lib"
)



# Include the folder so your #include statements can locate the headers
#target_include_directories(client_static PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx")

set_target_properties(client_static PROPERTIES
        POSITION_INDEPENDENT_CODE 1
)

if(NOT APPLE AND (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64"))

else()

    message(STATUS "📥 MacOS or Linux")

    # Global flag bypass for older compilers/headers if built on alternative environments
    add_compile_definitions(HAVE_XGENERICEVENTCOOKIE=1)


endif()



# ==============================================================================
# GENERAL INCLUDE DIRECTORIES
# ==============================================================================
target_include_directories(client_static PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

