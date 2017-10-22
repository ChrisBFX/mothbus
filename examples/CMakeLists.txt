cmake_minimum_required(VERSION 3.2)

find_package(Boost 1.54 REQUIRED COMPONENTS system program_options)
find_package(Threads REQUIRED)

add_executable(mothtcpserver tcpserver.cpp)
target_link_libraries(mothtcpserver mothbus ${Boost_LIBRARIES} Threads::Threads)

add_executable(mothtcpclient tcpclient.cpp)
target_link_libraries(mothtcpclient mothbus ${Boost_LIBRARIES} Threads::Threads)

