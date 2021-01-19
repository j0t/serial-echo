add_library(
    test-lib
    TestSerialServer.cpp
)

target_include_directories(test-lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(test-lib PRIVATE 
    "${CMAKE_SOURCE_DIR}/lib/includes"
    "${CMAKE_SOURCE_DIR}/test/includes"
)

target_link_libraries(test-lib PRIVATE Threads::Threads
    serial-echo-lib
)

add_executable(${TEST_NAME} test.cpp)

target_include_directories(${TEST_NAME} PRIVATE 
    "${CMAKE_SOURCE_DIR}/lib/includes"
    "${CMAKE_SOURCE_DIR}/test/includes"
)

target_link_libraries(${TEST_NAME} PRIVATE Threads::Threads 
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    test-lib
)
