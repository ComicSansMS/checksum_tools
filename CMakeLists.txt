cmake_minimum_required(VERSION 3.13)

project(checksum_tools)

find_package(Boost REQUIRED)
find_package(fmt REQUIRED)

add_executable(isdbsum isdbsum.cpp)
target_link_libraries(isdbsum PUBLIC fmt::fmt)
set_property(TARGET isdbsum PROPERTY CXX_STANDARD 17)
set_property(TARGET isdbsum PROPERTY CXX_STANDARD_REQUIRED ON)
add_executable(crc32sum crc32sum.cpp)
target_link_libraries(crc32sum PUBLIC Boost::boost fmt::fmt)
set_property(TARGET crc32sum PROPERTY CXX_STANDARD 17)
set_property(TARGET crc32sum PROPERTY CXX_STANDARD_REQUIRED ON)
