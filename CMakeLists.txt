cmake_minimum_required(VERSION 3.8)
PROJECT(mothbus)


if (MSVC)
	# multiprocess build + bigobj linker
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

file(GLOB_RECURSE PROJ_INCLUDES include/*.h)
file(GLOB_RECURSE PROJ_SRC src/*.cpp)

set(Boost_USE_STATIC_RUNTIME    OFF)
set(Boost_USE_STATIC_LIBS        ON)
find_package(Boost 1.63 REQUIRED COMPONENTS system)

if (MSVC)
    add_definitions (-DBOOST_ALL_NO_LIB) # disable autolinking
endif()

set(GSL_INCLUDE_DIRS "C:\\program files\\GSL\\include" CACHE PATH "Path to core guideline library")
include_directories(include ${Boost_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS})

add_library(mothbus ${PROJ_SRC} ${PROJ_INCLUDES})
target_include_directories(mothbus PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(mothbus ${Boost_LIBRARIES})

add_subdirectory(test)

option(Build_Examples OFF "Build examples for mothbus")
if (${Build_Examples})
	add_subdirectory(examples)
endif()