cmake_minimum_required(VERSION 3.22)

project(chadinterpreter VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include(GNUInstallDirs)

set(default_build_type "Debug")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

include(CheckSymbolExists)

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
        src/binary_ops.h
        src/unary_ops.h
        src/gc.h
        src/gc.c
        src/builtins.h
        src/builtins.c
        src/getopt_impl.h
        src/getline_impl.h
        src/errors.h
)

add_executable(chadeval src/eval.c)

target_compile_definitions(chadeval PRIVATE APP_VERSION="${PROJECT_VERSION}" APP_NAME="${PROJECT_NAME}")

check_symbol_exists(getopt "unistd.h" HAVE_GETOPT)
check_symbol_exists(getline "stdio.h" HAVE_GETLINE)

if (HAVE_GETLINE)
    target_compile_definitions(chadinterpreter PRIVATE HAVE_GETLINE)
endif ()

if (HAVE_GETOPT)
    target_compile_definitions(chadeval PRIVATE HAVE_GETOPT)
endif ()

target_link_libraries(chadeval PRIVATE chadinterpreter)

if (UNIX)
    target_compile_options(chadinterpreter PRIVATE "-Wall")
    target_compile_options(chadeval PRIVATE "-Wall")
endif ()

install(TARGETS chadinterpreter chadeval)