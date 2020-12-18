
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

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads ${Boost_LIBRARIES})

add_executable(${TEST_NAME} test.cpp)
target_link_libraries(${TEST_NAME} PRIVATE Threads::Threads 
    ${Boost_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
