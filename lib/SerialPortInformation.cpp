#include "SerialPortInformation.h"

SerialPortInformation::SerialPortInformation(int argc, char *argv[])
{
    using namespace boost::program_options;

    const char *HELP = "help";
    const char *PORT = "port";
    const char *BAUD_RATE = "baud_rate";
    const char* DEBUG_LEVEL = "debug_level";

    options_description description("Options");
    description.add_options()
        (HELP, "show help message")
        (PORT, value<std::string>(&this->portName)->default_value("/dev/nmp1"), "set serial port")
        (BAUD_RATE, value<unsigned long>(&this->baudRate)->default_value(1200), "set baud rate")
        (DEBUG_LEVEL, value<unsigned int>(&this->debugLevel)->default_value(1), "set debug level (0 - none, 1 - full)")
    ;

    variables_map variableMap;
    store(parse_command_line(argc, argv, description), variableMap);
    notify(variableMap);

    if (variableMap.count(HELP))
    {
        std::cout << description << "\n";
    }

    if (variableMap.count(PORT))
    {
        std::cout << "Serial port device was set to " << variableMap[PORT].as<std::string>() << "\n";
    }
    else
    {
        std::cout << "Serial port device was set to default\n";
    }

    if (variableMap.count(BAUD_RATE))
    {
        std::cout << "Serial port device baud rate was set to " << variableMap[BAUD_RATE].as<unsigned long>() << "\n";
    }
    else
    {
        std::cout << "Serial device baud rate was set to default\n";
    }

    if (variableMap.count(DEBUG_LEVEL))
    {
        std::cout << "Debug level was set to " << variableMap[DEBUG_LEVEL].as<unsigned int>() << "\n";
    }
    else
    {
        std::cout << "Debug level was set to default\n";
    }
};
