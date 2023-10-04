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

find_package(PkgConfig)

add_library(pnsinterpreter
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

if (PKG_CONFIG_FOUND)
    pkg_check_modules(readline IMPORTED_TARGET readline)

    if (readline_FOUND)
        add_executable(repl src/repl.c)
        target_link_libraries(repl PRIVATE pnsinterpreter PkgConfig::readline)
        if (UNIX)
            target_compile_options(repl PRIVATE "-Wall")
        endif ()
    endif ()

    pkg_check_modules(GTK4 IMPORTED_TARGET gtk4)

    if (GTK4_FOUND)
        add_executable(gui src/gui.c)
        target_link_libraries(gui PRIVATE pnsinterpreter PkgConfig::GTK4)
        if (UNIX)
            target_compile_options(gui PRIVATE "-Wall")
        endif ()
    endif ()
endif ()

target_link_libraries(eval PRIVATE pnsinterpreter)

if (UNIX)
    target_compile_options(pnsinterpreter PRIVATE "-Wall")
    target_compile_options(eval PRIVATE "-Wall")
endif ()
