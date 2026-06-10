cmake_minimum_required(VERSION 3.16)
project(MServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Параллельная сборка
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
    message(STATUS "Building with ${N} parallel jobs")
endif()

# Поиск Boost
find_package(Boost 1.65.0 REQUIRED COMPONENTS system filesystem thread)

# Поиск SQLiteCpp
find_path(SQLITECPP_INCLUDE_DIR SQLiteCpp.h
    PATHS /usr/include /usr/local/include
    PATH_SUFFIXES SQLiteCpp
)

find_library(SQLITECPP_LIBRARY
    NAMES SQLiteCpp
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
)

# Поиск SQLite3 (зависимость SQLiteCpp)
find_library(SQLITE3_LIB sqlite3)
find_package(OpenSSL REQUIRED)

if(SQLITECPP_INCLUDE_DIR AND SQLITECPP_LIBRARY)
    set(SQLITECPP_FOUND TRUE)
    message(STATUS "Found SQLiteCpp: ${SQLITECPP_LIBRARY}")
    message(STATUS "SQLiteCpp include: ${SQLITECPP_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "SQLiteCpp not found! Install: sudo apt install libsqlitecpp-dev")
endif()

if(NOT SQLITE3_LIB)
    message(FATAL_ERROR "SQLite3 not found! Install: sudo apt install libsqlite3-dev")
endif()

# Включаем все исходники автоматически
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h")

# Создаем исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Директории для включения
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/websocketpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SQLITECPP_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

# Библиотеки для линковки - ВАЖНЫЙ ПОРЯДОК!
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SQLITECPP_LIBRARY}  # Сначала SQLiteCpp
    ${SQLITE3_LIB}        # Потом SQLite3
    ${Boost_LIBRARIES}
    OpenSSL::Crypto
    pthread
    dl
)

# Выходная директория
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
message(STATUS "SQLiteCpp library: ${SQLITECPP_LIBRARY}")
message(STATUS "SQLite3 library: ${SQLITE3_LIB}")