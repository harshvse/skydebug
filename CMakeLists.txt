cmake_minimum_required(VERSION 3.15)

project("skydebug" VERSION 0.1 DESCRIPTION "A Native debugger to debug compiled languages" LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(readline REQUIRED IMPORTED_TARGET readline)

include(CTest)

add_subdirectory(src)
add_subdirectory(tools)



if(BUILD_TESTING)
    find_package(Catch2 CONFIG REQUIRED)
    add_subdirectory(test)
endif()
