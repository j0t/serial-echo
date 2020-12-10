
cmake_minimum_required(VERSION 3.14)
project(serial-echo VERSION 1.0)

find_package (Boost COMPONENTS thread program_options REQUIRED)

if (Boost_FOUND)
    message(STATUS "Boost is found!")
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions("-DHAS_BOOST")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads ${Boost_LIBRARIES})
