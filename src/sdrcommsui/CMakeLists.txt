
# Define your static library target first
add_executable(sdrcommsui
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx/SDL2_framerate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx/SDL2_framerate.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx/SDL2_gfxPrimitives.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx/SDL2_gfxPrimitives.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx/SDL2_imageFilter.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx/SDL2_imageFilter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx/SDL2_rotozoom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx/SDL2_rotozoom.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx/SDL2_gfxPrimitives_font.h
        main.cpp
        netclient.cpp
        include/netclient.h

)
include_directories(${cli11_SOURCE_DIR}/include)
# Core link layer: Always link against your static drawing engine target
target_link_libraries(sdrcommsui PRIVATE client_static)


# Include the folder so your #include statements can locate the headers
target_include_directories(sdrcommsui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/SDL2_gfx")


set_target_properties(sdrcommsui PROPERTIES
        POSITION_INDEPENDENT_CODE 1
)

# ==============================================================================
# SDL2 CONFIGURATION SPLIT
# ==============================================================================
if (EMSCRIPTEN)
    # --------------------------------------------------------------------------
    # TARGET: WebAssembly (Emscripten Browser App)
    # Use Emscripten's built-in system SDL2 ports cache
    # --------------------------------------------------------------------------
    message(STATUS "🌐 TARGET: WebAssembly -> Routing via Emscripten system ports")

    # Instruct Emscripten to inject the native browser ports definition
    target_compile_options(sdrcommsui PUBLIC "-sUSE_SDL=2")

elseif (NOT APPLE AND (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64") AND NOT CMAKE_CROSSCOMPILING)
    # --------------------------------------------------------------------------
    # TARGET: Linux AMD64 Native Desktop
    # Use the system-deployed static libSDL2.a instead of building from source
    # --------------------------------------------------------------------------
    message(STATUS "📦 Native AMD64: Linking against system static libSDL2.a")

    # Safely look for the system package using CMake's standard config modules
    find_package(SDL2 REQUIRED)

    # Link your target against the system discovered static target
    target_link_libraries(sdrcommsui PUBLIC SDL2::SDL2-static)

    # Map headers to the system path
    target_include_directories(sdrcommsui PUBLIC ${SDL2_INCLUDE_DIRS})
elseif (NOT APPLE AND (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64"))
    # --------------------------------------------------------------------------
    # TARGET: Linux AMD64 Native Desktop
    # Use the system-deployed static libSDL2.a instead of building from source
    # --------------------------------------------------------------------------
    message(STATUS "📦 Native ARM64: Linking against system dynamic libSDL2")

    # Safely look for the system package using CMake's standard config modules
    find_package(SDL2 REQUIRED)

    # Link your target against the system discovered static target
    target_link_libraries(sdrcommsui PUBLIC SDL2::SDL2)

    # Map headers to the system path
    target_include_directories(sdrcommsui PUBLIC ${SDL2_INCLUDE_DIRS})
else ()
    # --------------------------------------------------------------------------
    # TARGET: macOS / iOS Handheld Cross-Compilation
    # Clone and build SDL2 statically from source
    # --------------------------------------------------------------------------
    message(STATUS "📥 MacOS: Fetching SDL2 Source via FetchContent")

    # Global flag bypass for older compilers/headers if built on alternative environments
    add_compile_definitions(HAVE_XGENERICEVENTCOOKIE=1)

    if (NOT APPLE)
        # Strictly disable desktop servers for the Anbernic hardware target
        set(VIDEO_X11 OFF CACHE BOOL "Disable X11 for embedded" FORCE)
        set(VIDEO_WAYLAND OFF CACHE BOOL "Disable Wayland for embedded" FORCE)
        set(VIDEO_KMSDRM ON CACHE BOOL "Enable KMSDRM for embedded" FORCE)
    endif ()

    include(FetchContent)
    FetchContent_Declare(
            SDL2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-2.30.11
    )

    # Force SDL's build tree to generate static libraries
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL2)

    # Link against the freshly compiled static target
    target_link_libraries(sdrcommsui PUBLIC SDL2::SDL2-static)

    # Map headers to the downloaded source directory inclusion tree
    target_include_directories(sdrcommsui PUBLIC ${sdl2_SOURCE_DIR}/include)
endif ()


