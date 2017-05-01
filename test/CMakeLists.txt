cmake_minimum_required(VERSION 3.8)
include(CTest)

enable_testing()

file(GLOB PROJ_SRC *.cpp)

find_package(GTest REQUIRED)

add_executable(mothtest ${PROJ_SRC})
target_link_libraries(mothtest mothbus GTest::GTest GTest::Main)

GTEST_ADD_TESTS(mothtest "" ${PROJ_SRC})
