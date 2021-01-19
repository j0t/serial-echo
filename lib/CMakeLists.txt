add_library(
    serial-echo-lib
    SerialPortInformation.cpp
    SerialServer.cpp
    utility.cpp
    Logger.cpp
)

target_include_directories(serial-echo-lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
