cmake_minimum_required(VERSION 3.14)
project(serial-echo VERSION 1.0)

set(TEST_NAME "${PROJECT_NAME}-test")

find_package (Boost COMPONENTS system filesystem thread program_options unit_test_framework REQUIRED)

if (Boost_FOUND)
    message(STATUS "Boost is found!")
    include_directories(${Boost_INCLUDE_DIR})
    include_directories (${TEST_SOURCE_DIR}/src)
    add_definitions("-DHAS_BOOST")
    add_definitions ("-DBOOST_TEST_DYN_LINK")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(lib)
add_subdirectory(test)

target_include_directories(serial-echo-lib PRIVATE 
    "${CMAKE_SOURCE_DIR}/lib/includes"
)

target_link_libraries(serial-echo-lib PRIVATE ${Boost_LIBRARIES})

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_SOURCE_DIR}/lib/includes"
)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads serial-echo-lib)

enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
