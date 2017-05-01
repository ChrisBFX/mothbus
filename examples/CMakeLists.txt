cmake_minimum_required(VERSION 3.8)

find_package(Boost 1.63 REQUIRED COMPONENTS system program_options)


add_executable(mothtcpserver tcpserver.cpp)
target_link_libraries(mothtcpserver mothbus ${Boost_LIBRARIES})

add_executable(mothtcpclient tcpclient.cpp)
target_link_libraries(mothtcpclient mothbus ${Boost_LIBRARIES})

