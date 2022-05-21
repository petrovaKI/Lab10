cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(
        HUNTER_CACHE_SERVERS
        "https://github.com/bmstu-iu8-cpp-sem-3/hunter-cache"
        CACHE STRING "Default cache server"
)

include("cmake/HunterGate.cmake")

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.314.tar.gz"
        SHA1 "95c47c92f68edb091b5d6d18924baabe02a6962a"
)

project(dbcs)
string(APPEND CMAKE_CXX_FLAGS " -pedantic -Werror -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS " -Wno-unused-command-line-argument")
string(APPEND CMAKE_CXX_FLAGS " -Wshadow -Wnon-virtual-dtor")

hunter_add_package(
        Boost
        COMPONENTS
        program_options
        thread
        log
        filesystem
)
find_package(
        Boost
        CONFIG REQUIRED
        program_options
        thread
        log
        log_setup
        filesystem
)
hunter_add_package(rocksdb)
find_package(RocksDB CONFIG REQUIRED)
find_package(Threads)

add_executable(
        ${PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/InputBD.cpp
        sources/logging.cpp
        sources/OutpCalculation.cpp
)
add_executable(demo
        ${CMAKE_CURRENT_SOURCE_DIR}/demo/main.cpp
        )
target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Third-party/PicoSHA>"
        "$<INSTALL_INTERFACE:Third-party/PicoSHA>")
target_link_libraries(
        ${PROJECT_NAME}
        Boost::program_options
        Boost::thread
        Boost::log
        Boost::log_setup
        Boost::filesystem
        RocksDB::rocksdb
)