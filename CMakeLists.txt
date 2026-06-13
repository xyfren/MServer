cmake_minimum_required(VERSION 3.24)
project(MServer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(nlohmann_json REQUIRED)

file(GLOB_RECURSE SOURCES "network/*.cpp" "controller/*.cpp" "domain/*.cpp" "data/*.cpp" "main.cpp")

add_executable(MServer ${SOURCES})

target_include_directories(MServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(MServer PRIVATE 
    Boost::boost 
    nlohmann_json::nlohmann_json
)