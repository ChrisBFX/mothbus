cmake_minimum_required(VERSION 3.2)
include(CTest)



file(GLOB PROJ_SRC *.cpp)

find_package(GTest REQUIRED)
include_directories(include ${GTEST_INCLUDE_DIRS})
add_executable(mothtest ${PROJ_SRC})
set(THREADING_LIB "")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(THREADING_LIB pthread)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(THREADING_LIB pthread)
endif()
target_link_libraries(mothtest mothbus ${GTEST_BOTH_LIBRARIES} ${THREADING_LIB})

GTEST_ADD_TESTS(mothtest "" ${PROJ_SRC})
