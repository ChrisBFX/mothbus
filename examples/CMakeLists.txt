cmake_minimum_required(VERSION 3.2)

find_package(Boost 1.54 REQUIRED COMPONENTS system program_options)
set(THREADING_LIB "")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(THREADING_LIB pthread)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(THREADING_LIB pthread)
endif()

add_executable(mothtcpserver tcpserver.cpp)
target_link_libraries(mothtcpserver mothbus ${Boost_LIBRARIES} ${THREADING_LIB})

add_executable(mothtcpclient tcpclient.cpp)
target_link_libraries(mothtcpclient mothbus ${Boost_LIBRARIES} ${THREADING_LIB})

