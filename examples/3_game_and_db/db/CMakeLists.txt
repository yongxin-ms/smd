cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME Db)
PROJECT(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_UNITY_BUILD yes)
set(CMAKE_UNITY_BUILD_BATCH_SIZE 16)

if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -g -pthread")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../gen/
	${CMAKE_CURRENT_SOURCE_DIR}/../../../include
	)
	
file(GLOB SELF_TEMP_SRC_FILES
	"*.cpp"
	"*.h"
	"../server_common/*.h"
	"../gen/*.h"
	"../gen/*.cpp"
	)
source_group(src FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES
	"../../../../include/*.h"
	)
source_group(include FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})
	
file(GLOB SELF_TEMP_SRC_FILES
	"../../../../include/common/*.h"
	)
source_group(include\\common FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES
	"../../../../include/container/*.h"
	)
source_group(include\\container FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES
	"../../../../include/mem_alloc/*.h"
	)
source_group(include\\mem_alloc FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

add_executable(${PROJECT_NAME} ${SELF_SRC_FILES})