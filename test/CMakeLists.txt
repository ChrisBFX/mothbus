cmake_minimum_required(VERSION 3.2)
include(CTest)

enable_testing()

file(GLOB PROJ_SRC *.cpp)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
include_directories(include ${GTEST_INCLUDE_DIRS})
add_executable(mothtest ${PROJ_SRC})

target_link_libraries(mothtest mothbus ${GTEST_BOTH_LIBRARIES} Threads::Threads)

GTEST_ADD_TESTS(mothtest "" ${PROJ_SRC})
