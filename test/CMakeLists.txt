cmake_minimum_required(VERSION 2.8)
project(m3bp-test)

option(ENABLE_LOCALITY "Enable locality support (hwloc is required)" On)

# do not install test-related components
macro(install)
endmacro(install)

add_subdirectory(googletest)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

find_package(Threads REQUIRED)

if(ENABLE_LOCALITY)
	find_package(hwloc REQUIRED)
	if(HWLOC_FOUND)
		add_definitions(-DM3BP_LOCALITY_ENABLED)
	endif()
endif()

find_package(Boost 1.55 COMPONENTS log REQUIRED)
add_definitions(-DBOOST_LOG_DYN_LINK)

include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(.)
include_directories(../include)
include_directories(../src)

file(GLOB TEST_SOURCES
     "main.cpp"
     "api/*.cpp"
     "context/*.cpp"
     "util/*.cpp"
     "scheduler/*.cpp"
     "memory/*.cpp"
     "graph/*.cpp"
     "system/*.cpp"
     "tasks/*.cpp")

if(UNIX)
	set(COMPILE_OPTIONS "-std=c++11 -g -Wall -Wextra")
	set(LINK_OPTIONS    "-pthread")
endif()

add_executable(m3bp-test ${TEST_SOURCES})
set_target_properties(m3bp-test PROPERTIES COMPILE_FLAGS ${COMPILE_OPTIONS})
set_target_properties(m3bp-test PROPERTIES LINK_FLAGS    ${LINK_OPTIONS})
set_target_properties(m3bp PROPERTIES INTERPROCEDURAL_OPTIMIZATION On)
target_link_libraries(m3bp-test m3bp gtest ${Boost_LIBRARIES})

