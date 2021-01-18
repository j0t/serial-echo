#pragma once

#include <boost/program_options.hpp>

#include "utility.h"

struct SerialPortInformation
{
public:
    std::string portName;
    unsigned long baudRate;
    unsigned int debugLevel;

public:
    SerialPortInformation() = default;
    SerialPortInformation(int argc, char *argv[]);
};
