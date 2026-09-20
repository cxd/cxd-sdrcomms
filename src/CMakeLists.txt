add_subdirectory(client)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" AND NOT IOS)
    # compiling on arm.

else()
    if(NOT IOS)
        # compiling macos and linux

    endif()
endif()
