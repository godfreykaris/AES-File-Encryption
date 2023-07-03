					   
cmake_minimum_required(VERSION 3.22.1)

# set the project name and version
project(Cipher VERSION 1.0)

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Cipher)
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

include_directories("${CMAKE_SOURCE_DIR}")

FILE(GLOB_RECURSE MY_HEADERS "include/*.h")
FILE(GLOB MY_SOURCES "src/*.cpp" "resources/*.ico" "resources/*.rc")
# add the executables
add_executable(Cipher ${MY_HEADERS} ${MY_SOURCES})

target_include_directories(Cipher PUBLIC "${PROJECT_SOURCE_DIR}/include")	
										
# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

					   