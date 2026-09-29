add_subdirectory(client)
add_subdirectory(application)
add_subdirectory(sdrcommsui)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" AND NOT IOS)
    # compiling on arm.

else()
    if(NOT IOS)
        # compiling macos and linux

    endif()
endif()
