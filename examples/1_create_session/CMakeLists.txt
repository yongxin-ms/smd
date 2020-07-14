cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME create_session)
PROJECT(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 11)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	message(debug_flag2:${debug})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall -g")
	find_program(CCACHE_FOUND ccache)
	if (CCACHE_FOUND)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
	endif(CCACHE_FOUND)
	add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
	message (debug_flag3:${debug})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../include
	)

set(SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/util.h
	)
	
set(INCLUDE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/smd.h
	)
	
set(COMMON_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/common/smd_defines.h
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/common/slice.h
	)
	
set(CONTAINER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/container/hash.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/container/list.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/container/string.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/container/base_obj.hpp
	)

set(MEM_ALLOC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/mem_alloc/buddy.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/mem_alloc/alloc.hpp
	)

source_group(source FILES ${SOURCE_FILES})
source_group(include FILES ${INCLUDE_FILES})

source_group(include\\common FILES ${COMMON_FILES})
source_group(include\\container FILES ${CONTAINER_FILES})
source_group(include\\mem_alloc FILES ${MEM_ALLOC_FILES})

add_definitions(-DASIO_STANDALONE)

add_executable(${PROJECT_NAME}
	${SOURCE_FILES}
	${INCLUDE_FILES}
	${COMMON_FILES}
	${CONTAINER_FILES}
	${MEM_ALLOC_FILES}
	)

