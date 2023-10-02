cmake_minimum_required(VERSION 3.22)

project(pns-interpreter LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(default_build_type "Debug")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

file(GLOB_RECURSE SRCS src/*.c)
file(GLOB_RECURSE HDRS src/*.h)

add_executable(eval ${SRCS} ${HDRS})

if (UNIX)
  target_compile_options(eval PRIVATE "-Wall")
endif ()

install(TARGETS eval)
