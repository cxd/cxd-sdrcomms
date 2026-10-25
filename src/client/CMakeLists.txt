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
        GIT_TAG master
)

# CRITICAL FIX A: Inform the build system that SoapySDR is active inside this source tree.
# This prevents it from escaping out to /opt/homebrew configurations.
FetchContent_GetProperties(soapysdr)
if (NOT soapysdr_POPULATED)
    FetchContent_Populate(soapysdr)
    set(SOAPY_SDR_IN_TREE_SOURCE_DIR "${soapysdr_SOURCE_DIR}" CACHE INTERNAL "")
    add_subdirectory(${soapysdr_SOURCE_DIR} ${soapysdr_BINARY_DIR})
endif ()

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
        GIT_TAG master
)


# Tell the remote build explicitly to look AT the fetched binary directory first,
# and tell find_package to ignore global paths (/opt/homebrew or /usr) completely.
set(SoapySDR_DIR "${soapysdr_BINARY_DIR}" CACHE PATH "" FORCE)
set(CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY ON)
set(CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY ON)

FetchContent_MakeAvailable(soapyremote)

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan found:")
find_package(glslang REQUIRED)
message(STATUS "glslang found:")

if (APPLE)
    find_path(MOSQUITTO_INCLUDE_DIR
            NAMES mosquitto.h
            HINTS ${HOMEBREW_HINTS}
            PATH_SUFFIXES include)
    find_library(MOSQUITTO_LIBRARY
            NAMES mosquitto
            HINTS ${HOMEBREW_HINTS}
            PATH_SUFFIXES lib
            )
    message(STATUS "mosquitto found:")
    find_path(ONNXRUNTIME_INCLUDE_DIR
            NAMES onnxruntime
            HINTS ${HOMEBREW_HINTS}
            PATH_SUFFIXES include)
    find_library(ONNX_RUNTIME_LIBRARY
            NAMES onnxruntime
            HINTS ${HOMEBREW_HINTS}
            PATH_SUFFIXES lib
    )
    message(STATUS "onnxruntime found:")
else()
    find_path(MOSQUITTO_INCLUDE_DIR
            NAMES mosquitto.h
            HINTS /usr/
            PATH_SUFFIXES include)
    find_library(MOSQUITTO_LIBRARY
            NAMES mosquitto
            HINTS /usr/
            PATH_SUFFIXES lib
    )
    message(STATUS "mosquitto found:")
    find_path(ONNXRUNTIME_INCLUDE_DIR
            NAMES onnxruntime_cxx_api.h
            HINTS /usr/
            PATH_SUFFIXES include)
    # x86_64-linux-gnu/
    find_library(ONNX_RUNTIME_LIBRARY
            NAMES onnxruntime
            HINTS /usr/
            PATH_SUFFIXES lib
    )
    message(STATUS "onnxruntime found:")
endif()
# vKFFT
FetchContent_Declare(
        vkfft
        GIT_REPOSITORY https://github.com/DTolm/VkFFT.git
        GIT_TAG v1.3.4
)
FetchContent_MakeAvailable(vkfft)
if(APPLE)
    target_compile_definitions(VkFFT INTERFACE VKFFT_BACKEND=0)
endif()

# Json library

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.12.0)
FetchContent_MakeAvailable(json)


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
        include/NetIQBlock.h
        FFTTransform.cpp
        include/FFTTransform.h
        FFTSampleProcessor.cpp
        include/FFTSampleProcessor.h
        ChunkHeader.cpp
        include/ChunkHeader.h
        MQTTServer.cpp
        include/MQTTServer.h
        include/MessageQueue.h
        include/ControlMessage.h
        ModelFileSpec.cpp
        include/ModelFileSpec.h
        ModelFileLoader.cpp
        include/ModelFileLoader.h)

# 2. If the option is turned ON, inject a preprocessor macro into the C++ compiler
if (ENABLE_SDR_DEBUG)
    target_compile_definitions(client_static PRIVATE CL_DEBUG_PLUTO=1)
    message(STATUS ">>> PlutoSDR Conditional Debugging: ENABLED <<<")
endif ()


# 1. Manually pull the source directory variable from the fetched SoapySDR configuration
FetchContent_GetProperties(soapysdr)
if (soapysdr_POPULATED)
    # 2. Force export the raw header folder path downstream.
    # Use PUBLIC so it applies to both this library AND your main application target.
    target_include_directories(client_static PUBLIC
            "${soapysdr_SOURCE_DIR}/include"
    )
endif ()

target_link_libraries(client_static PRIVATE SoapySDR)

# 2. Tell CMake to build the remoteSupport plugin whenever my_radio_app is built
add_dependencies(client_static remoteSupport)

# 3. Automatically inject runtime search paths for your IDE/Debugger
# This embeds the built SoapySDR library directory into your executable path info
set_target_properties(client_static PROPERTIES
        BUILD_RPATH "${soapysdr_BINARY_DIR}/lib"
)

# Modern CMake approach: link include directories and library
target_include_directories(client_static PRIVATE ${MOSQUITTO_INCLUDE_DIR})
target_link_libraries(client_static PRIVATE ${MOSQUITTO_LIBRARY})
# link with onnxruntime
target_include_directories(client_static PRIVATE ${ONNXRUNTIME_INCLUDE_DIR})
target_link_libraries(client_static PRIVATE ${ONNX_RUNTIME_LIBRARY})

if (APPLE)
    target_compile_definitions(client_static PRIVATE VKFFT_BACKEND=0)
    # Append the nested include path for glslang
    target_include_directories(client_static PUBLIC "/opt/homebrew/include")
    target_include_directories(client_static PUBLIC "/opt/homebrew/include/glslang/Include")
    target_link_directories(client_static PUBLIC /opt/homebrew/lib)


else()
    target_include_directories(client_static PUBLIC /usr/include)
    target_include_directories(client_static PUBLIC /usr/include/glslang/Include/)

endif ()

FetchContent_GetProperties(vkfft)
if (vkfft_POPULATED)
    target_include_directories(client_static PUBLIC
            "${vkfft_SOURCE_DIR}/vkFFT"
    )
    if (APPLE)
        target_compile_definitions(client_static PRIVATE VKFFT_BACKEND=0)
        target_link_libraries(client_static PRIVATE "-framework Foundation" "-framework Metal")
        target_link_libraries(client_static PUBLIC Vulkan::Vulkan)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        message(STATUS "Configure vkFFT for Vulkan backend")


        target_compile_definitions(client_static PRIVATE VKFFT_BACKEND=0)
        target_link_libraries(client_static PUBLIC Vulkan::Vulkan)
    endif ()
    #target_link_libraries(client_static PRIVATE vkfft)
    target_link_libraries(client_static PUBLIC
            glslang::glslang
            glslang::SPIRV
   )

endif ()



target_link_libraries(client_static PUBLIC nlohmann_json::nlohmann_json)
# Include the folder so your #include statements can locate the headers
#target_include_directories(client_static PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx")

set_target_properties(client_static PROPERTIES
        POSITION_INDEPENDENT_CODE 1
)

if (NOT APPLE AND (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64"))

else ()

    message(STATUS "📥 MacOS or Linux")

    # Global flag bypass for older compilers/headers if built on alternative environments
    add_compile_definitions(HAVE_XGENERICEVENTCOOKIE=1)


endif ()


# ==============================================================================
# GENERAL INCLUDE DIRECTORIES
# ==============================================================================
target_include_directories(client_static PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

