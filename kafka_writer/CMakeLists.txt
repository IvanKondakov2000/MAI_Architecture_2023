cmake_minimum_required(VERSION 3.2)

project(hl_service_mesh C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(REDISCPP_FLAGS "-DREDISCPP_HEADER_ONLY=ON")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall ${REDISCPP_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -Wall -DNDEBUG ${REDISCPP_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(KAFKA_WRITER "kafka_writer")

find_package(OpenSSL)
find_package(Threads)
find_package(ZLIB)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL)
find_package(CppKafka REQUIRED)

if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories(${Poco_INCLUDE_DIRS})
include_directories("/usr/include/mysql")
include_directories("/usr/local/include/mysql")
link_directories("/usr/local/lib")

add_executable(${KAFKA_WRITER} writer_main.cpp
                               ../common/config.cpp
                               ../common/database.cpp
                               ../auth_service/src/db/cache.cpp
                               ../auth_service/src/db/user.cpp)
target_include_directories(${KAFKA_WRITER} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${KAFKA_WRITER} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${KAFKA_WRITER} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${KAFKA_WRITER} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(${KAFKA_WRITER} PRIVATE
                             ${CMAKE_THREAD_LIBS_INIT}
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "rdkafka++"
                             CppKafka::cppkafka
                             "mysqlclient"
                             ZLIB::ZLIB)
