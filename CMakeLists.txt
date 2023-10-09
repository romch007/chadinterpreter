cmake_minimum_required(VERSION 3.22)

project(chadinterpreter LANGUAGES C)

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

option(USE_JEMALLOC "Use jemalloc for heap allocation" OFF)

find_package(PkgConfig REQUIRED)

add_library(chadinterpreter
        src/ast.c
        src/ast.h
        src/cvector.h
        src/hashmap.c
        src/hashmap.h
        src/interpreter.c
        src/interpreter.h
        src/lexer.c
        src/lexer.h
        src/mem.c
        src/mem.h
        src/parser.c
        src/parser.h
        src/runtime_types.h
        src/tokens.h
        src/gc.h
        src/gc.c
)

add_executable(eval src/eval.c)


if (USE_JEMALLOC)
    pkg_check_modules(jemalloc REQUIRED IMPORTED_TARGET jemalloc)
    target_link_libraries(eval PRIVATE PkgConfig::jemalloc)
    target_compile_definitions(eval PRIVATE USE_JEMALLOC)
endif ()

target_link_libraries(eval PRIVATE chadinterpreter)

if (UNIX)
    target_compile_options(chadinterpreter PRIVATE "-Wall")
    target_compile_options(eval PRIVATE "-Wall")
endif ()
