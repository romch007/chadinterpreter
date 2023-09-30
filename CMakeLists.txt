cmake_minimum_required(VERSION 3.22)

project(pns-interpreter LANGUAGES C)

file(GLOB_RECURSE SRCS src/*.c)
file(GLOB_RECURSE HDRS src/*.h)

add_executable(eval ${SRCS} ${HDRS})

install(TARGETS eval)
