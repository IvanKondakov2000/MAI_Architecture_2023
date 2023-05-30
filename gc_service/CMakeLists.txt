cmake_minimum_required(VERSION 3.2)

project(hl_service_mesh C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GROUP_SERVICE "group_service")

find_package(OpenSSL)
find_package(Threads)
find_package(ZLIB)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL)


if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories(${Poco_INCLUDE_DIRS})
include_directories("/usr/include/mysql")
include_directories("/usr/local/include/mysql")
link_directories("/usr/local/lib")

add_executable(${GROUP_SERVICE} src/main.cpp
                                ../common/config.cpp
                                ../common/database.cpp
                                src/db/gcm.cpp
                                src/db/message.cpp)
target_include_directories(${GROUP_SERVICE} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${GROUP_SERVICE} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${GROUP_SERVICE} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${GROUP_SERVICE} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(${GROUP_SERVICE} PRIVATE
                             ${CMAKE_THREAD_LIBS_INIT}
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "mysqlclient"
                             ZLIB::ZLIB)
