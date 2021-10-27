cmake_minimum_required(VERSION 3.16.3)

message(STATUS "******************************************************************")
message(STATUS "CMAKE_SOURCE_DIR->" ${CMAKE_SOURCE_DIR})
message(STATUS "PROJECT_SOURCE_DIR->)" ../../..)
message(STATUS "******************************************************************")

project(Application)

find_package(Git)

function(REDEFINE_FILE_MACRO targetname)
	#获取当前目标的所有源文件
	get_target_property(SOURCE_FILE "${targetname}" SOURCES)
	#遍历源文件
	foreach(sourcefile ${SOURCE_FILE})
		#获取当前源文件的编译参数
		get_property(defs SOURCE "${sourcefile}"
				PROPERTY COMPILE_DEFINITIONS)
		#获取当前文件的绝对路径
		get_filename_component(filepath "${sourcefile}" ABSOLUTE)
		#将绝对路径中的项目路径替换成空,得到源文件相对于项目路径的相对路径
		string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
		#将我们要加的编译参数(__FILE__定义)添加到原来的编译参数里面
		list(APPEND defs "__FILE__=\"${relpath}\"")
		#重新设置源文件的编译参数
		set_property(
				SOURCE "${sourcefile}"
				PROPERTY COMPILE_DEFINITIONS ${defs}
		)
	endforeach()
endfunction()

execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format="%h"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE  GIT_REPO_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "GIT_REPOSITORY_VERSION ${GIT_REPO_VERSION}")

set(CMAKE_CXX_STANDARD 14)
set(GUI_XML)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Lib/)


SET(CMAKE_C_COMPILER "gcc")
SET(CMAKE_CXX_COMPILER "g++")
#SET(CMAKE_CXX_FLAGS "-std=c++14")

#add_definitions(-D__LOG_CONSOLE -D__GNU_SOURCE)

include_directories(${PROJECT_SOURCE_DIR}/Include)
include_directories(${PROJECT_SOURCE_DIR}/Include/uuid)
include_directories(${PROJECT_SOURCE_DIR}/Include/json)
include_directories(${PROJECT_SOURCE_DIR}/Include/Utils)
include_directories(${PROJECT_SOURCE_DIR}/Include/Infra)
include_directories(${PROJECT_SOURCE_DIR}/Include/curl)
include_directories(${PROJECT_SOURCE_DIR}/Include/ssl)
include_directories(${PROJECT_SOURCE_DIR}/Include/HttpClient)
include_directories(${PROJECT_SOURCE_DIR}/Include/Logger)
include_directories(${PROJECT_SOURCE_DIR}/Include/spdlog)
include_directories(${PROJECT_SOURCE_DIR}/Include/System)
include_directories(${PROJECT_SOURCE_DIR}/Include/easySqllite)
include_directories(${PROJECT_SOURCE_DIR}/Include/Storage)
include_directories(${PROJECT_SOURCE_DIR}/Include/Client)


file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/Src/*.cpp
		${PROJECT_SOURCE_DIR}/Src/*.cc
		${PROJECT_SOURCE_DIR}/Src/*.c)

link_directories(${PROJECT_SOURCE_DIR}/Lib/json)
link_directories(${PROJECT_SOURCE_DIR}/Lib/uuid)
link_directories(${PROJECT_SOURCE_DIR}/Lib/opencv)
link_directories(${PROJECT_SOURCE_DIR}/Lib/ssl)
link_directories(${PROJECT_SOURCE_DIR}/Lib/curl)
link_directories(${PROJECT_SOURCE_DIR}/Lib/spdlog)
link_directories(${PROJECT_SOURCE_DIR}/Lib/easySqllite)

add_definitions(-Wno-builtin-macro-redefined -D__GNU_SOURCE)

add_executable(neutron ${SRC_LIST})
REDEFINE_FILE_MACRO(neutron)

target_link_libraries(neutron
		spdlog crypto ssl curl 
		jsoncpp uuid easysqlite
        pthread
		)

add_compile_options(-Wall -fdata-sections -ffunction-sections -fstack-protector-all -frtti -fexceptions -D__GNU_SOURCE)

add_definitions(-DREPOSITORY_VERSION=${GIT_REPO_VERSION})





