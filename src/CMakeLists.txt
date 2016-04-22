cmake_minimum_required(VERSION 2.8)
project(m3bp)

set(M3BP_VERSION "0.1.1")
option(ENABLE_LOCALITY "Enable locality support (hwloc is required)" On)

set(M3BP_REVISION "$Format:%h$")
if(NOT ${M3BP_REVISION} MATCHES "^[0-9a-f]+$")
	execute_process(COMMAND git show -s --format=%h
	                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	                RESULT_VARIABLE GIT_SHOW_RESULT
	                OUTPUT_VARIABLE M3BP_REVISION
	                OUTPUT_STRIP_TRAILING_WHITESPACE
	                ERROR_QUIET)
	if(NOT ${GIT_SHOW_RESULT} EQUAL "0")
		set(M3BP_REVISION "exported")
	endif()
endif()
if(M3BP_REVISION)
	# set(M3BP_VERSION "${M3BP_VERSION}-${M3BP_REVISION}")
endif()

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

include_directories(${Boost_INCLUDE_DIR})
include_directories(${HWLOC_INCLUDE_DIR})
include_directories(.)
include_directories(../include)

file(GLOB SOURCES
     "api/*.cpp"
     "api/internal/*.cpp"
     "common/*.cpp"
     "context/*.cpp"
     "graph/*.cpp"
     "logging/*.cpp"
     "memory/*.cpp"
     "scheduler/*.cpp"
     "system/*.cpp"
     "tasks/*.cpp"
     "tasks/gather/*.cpp"
     "tasks/shuffle/*.cpp"
     "tasks/value_sort/*.cpp"
     "tasks/process/*.cpp")

if(UNIX)
	set(COMPILE_OPTIONS "-std=c++11 -g -Wall -Wextra -pthread")
	set(LINK_OPTIONS    "-pthread")
endif()

add_definitions(-DM3BP_VERSION="${M3BP_VERSION}")
add_library(m3bp SHARED ${SOURCES})
set_target_properties(m3bp PROPERTIES COMPILE_FLAGS ${COMPILE_OPTIONS})
set_target_properties(m3bp PROPERTIES LINK_FLAGS    ${LINK_OPTIONS})
set_target_properties(m3bp PROPERTIES VERSION       ${M3BP_VERSION})
set_target_properties(m3bp PROPERTIES INTERPROCEDURAL_OPTIMIZATION On)
target_link_libraries(m3bp
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${Boost_LIBRARIES}
                      ${HWLOC_LIBRARIES})
install(TARGETS m3bp LIBRARY DESTINATION lib)
install(DIRECTORY ../include/m3bp DESTINATION include)

