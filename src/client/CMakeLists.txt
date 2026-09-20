cmake_minimum_required(VERSION 3.5)
project(sdrcomms LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# --- Define standard search hints for macOS (Homebrew) and Linux ---
set(SDR_INCLUDE_HINTS
        /opt/homebrew/include
        /usr/include
        /usr/local/include
)

set(SDR_LIB_HINTS
        /opt/homebrew/lib
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
        /usr/local/lib
)


# --- 1. Find SoapySDR Core ---
find_path(SOAPY_SDR_INCLUDE_DIR NAMES SoapySDR/Version.h HINTS ${SDR_INCLUDE_HINTS})
find_library(SOAPY_SDR_LIBRARY NAMES SoapySDR HINTS ${SDR_LIB_HINTS})

# --- 2. Find SoapyRemote Client Library ---
find_path(SOAPY_REMOTE_INCLUDE_DIR NAMES SoapyRemoteDefs.hpp HINTS ${SDR_INCLUDE_HINTS})
find_library(SOAPY_REMOTE_LIBRARY NAMES SoapyRemote HINTS ${SDR_LIB_HINTS})

# --- Safety Verification ---
if(NOT SOAPY_SDR_LIBRARY OR NOT SOAPY_REMOTE_LIBRARY)
    message(FATAL_ERROR "Could not find SoapySDR or SoapyRemote! Please make sure the dev packages are installed on your system.")
endif()


add_library(client_static STATIC include/client.h
        client.cpp)


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

# Map headers dynamically depending on where they were discovered
target_include_directories(client_static PRIVATE
        ${SOAPY_SDR_INCLUDE_DIR}
        ${SOAPY_REMOTE_INCLUDE_DIR}
)

# Link against the discovered native system binaries (.dylib or .so)
target_link_libraries(client_static PRIVATE
        ${SOAPY_SDR_LIBRARY}
        ${SOAPY_REMOTE_LIBRARY}
)


# ==============================================================================
# GENERAL INCLUDE DIRECTORIES
# ==============================================================================
target_include_directories(client_static PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

