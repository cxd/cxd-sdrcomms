cmake_minimum_required(VERSION 3.10)


project(sdrcomms VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR})


list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(FetchContent)

# --- 1. Global Soapy Build Configurations ---
# Turn off native apps/tests to prevent target namespace clutter
set(ENABLE_APPS OFF CACHE INTERNAL "")
set(ENABLE_TESTS OFF CACHE INTERNAL "")

# Force uniform dynamic plugin architectures down-tree
set(BUILD_SHARED_LIBS ON CACHE INTERNAL "")

# --- 2. Declare and Populate SoapySDR Core ---
# 2. Fetch SoapySDR Core
FetchContent_Declare(
        soapysdr
        GIT_REPOSITORY https://github.com/pothosware/SoapySDR.git
        GIT_TAG        master
)

FetchContent_GetProperties(soapysdr)
if(NOT soapysdr_POPULATED)
    FetchContent_Populate(soapysdr)
    # Inject it strictly as an internal subdirectory target
    add_subdirectory("${soapysdr_SOURCE_DIR}" "${soapysdr_BINARY_DIR}")
endif()

if(APPLE)
    # 1. Allow CMake to recognize and unpack macOS .framework bundles natively
    set(CMAKE_FRAMEWORK_PATH "/Library/Frameworks" ${CMAKE_FRAMEWORK_PATH})

    # 2. Hard-define the include and library targets to clear the SoapyPluto checks
    set(LIBIIO_FOUND TRUE)
    set(LIBIIO_INCLUDE_DIRS "/Library/Frameworks/iio.framework/Headers")
    set(LIBIIO_LIBRARIES "-framework iio")

    set(LIBAD9361_FOUND TRUE)
    set(LIBAD9361_INCLUDE_DIRS "/Library/Frameworks/ad9361.framework/Headers")
    set(LIBAD9361_LIBRARIES "-framework ad9361")

    # Expose them to internal cache so downstream subdirectories inherit them
    set(LIBIIO_INCLUDE_DIR ${LIBIIO_INCLUDE_DIRS} CACHE PATH "" FORCE)
    set(LIBIIO_LIBRARY ${LIBIIO_LIBRARIES} CACHE STRING "" FORCE)
    set(LIBAD9361_INCLUDE_DIR ${LIBAD9361_INCLUDE_DIRS} CACHE PATH "" FORCE)
    set(LIBAD9361_LIBRARY ${LIBAD9361_LIBRARIES} CACHE STRING "" FORCE)

    # Downstream modules will now pass through the validation check smoothly
    if(LIBIIO_FOUND AND LIBAD9361_FOUND)
        FetchContent_Declare(
                soapypluto
                GIT_REPOSITORY https://github.com/pothosware/SoapyPlutoSDR.git
                GIT_TAG        master
        )
        FetchContent_GetProperties(soapypluto)
        if(NOT soapypluto_POPULATED)
            FetchContent_Populate(soapypluto)
            add_subdirectory(${soapypluto_SOURCE_DIR} ${soapypluto_BINARY_DIR})
        endif()
    else()
        message(WARNING "⚠️ libiio or libad9361 missing on host. Skipping SoapyPluto macOS module compilation.")
    endif()

else()

    # --- 4. Declare and Populate SoapyPluto Hardware Driver ---
    # NOTE: Host environment must have libiio development binaries installed!
    find_package(libiio QUIET)
    find_package(libad9361 QUIET)

    if(libiio_FOUND AND libad9361_FOUND)
        FetchContent_Declare(
                soapypluto
                GIT_REPOSITORY https://github.com/pothosware/SoapyPlutoSDR.git
                GIT_TAG        master
        )
        FetchContent_GetProperties(soapypluto)
        if(NOT soapypluto_POPULATED)
            FetchContent_Populate(soapypluto)
            add_subdirectory(${soapypluto_SOURCE_DIR} ${soapypluto_BINARY_DIR})
        endif()
    else()
        message(WARNING "⚠️ libiio or libad9361 missing on host. Skipping SoapyPluto server module compilation.")
    endif()

endif()


add_subdirectory(src)



# Define a standard base path if one isn't already specified (e.g., /usr/local)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation root directory" FORCE)
endif()
# --- Component A: Core Development Runtime (Libraries & Headers) ---
install(TARGETS SoapySDR
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        COMPONENT Development
)

# Export the headers downloaded via FetchContent so your system apps can see them globally
install(DIRECTORY "${soapysdr_SOURCE_DIR}/include/SoapySDR"
        DESTINATION include
        COMPONENT Development
)

# --- Component B: Network Server Daemon & Proxy Engine ---
if(TARGET SoapySDRServer)
    install(TARGETS SoapySDRServer
            RUNTIME DESTINATION bin
            COMPONENT Server
    )
endif()


# --- Component C: Dynamic Hardware & Network Plugin Drivers ---
# SoapySDR searches a strict, nested directory layout structure to load plugins
set(SOAPY_MODULES_LAYOUT "lib/SoapySDR_custom/modules0.8")

if(TARGET remoteSupport)
    install(TARGETS remoteSupport
            LIBRARY DESTINATION ${SOAPY_MODULES_LAYOUT}
            COMPONENT Development
    )
endif()

if(TARGET plutoSupport)
    install(TARGETS plutoSupport
            LIBRARY DESTINATION ${SOAPY_MODULES_LAYOUT}
            COMPONENT Server
    )
endif()