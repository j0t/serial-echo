#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

struct SerialPortInformation
{
    std::string portName;
    unsigned long baudRate;
    std::vector<char> sendString;
};

class SerialServer
{
private:
    boost::asio::io_context& io_context;
    SerialPortInformation portInformation;

    boost::asio::serial_port serialPort;
    boost::asio::streambuf dataBuffer;

public:
    SerialServer(boost::asio::io_context& io_context, SerialPortInformation portInformation)
        : io_context(io_context)
        , portInformation(portInformation)
        , serialPort(io_context, portInformation.portName)
    {
        setupPort(this->serialPort, this->portInformation.baudRate);
        startReadAndWrite();
    }

    void startReadAndWrite()
    {
        boost::system::error_code error;
        boost::asio::write(this->serialPort, boost::asio::buffer(this->portInformation.sendString, this->portInformation.sendString.size()), error);
        boost::asio::read_until(this->serialPort, this->dataBuffer, ' ', error);
    }

    const char* getBufferData()
    {
        std::istream inputStream(&this->dataBuffer);
        std::string dataString;
        inputStream >> dataString;

        const char * convertedString = dataString.c_str();

        return convertedString;
    }

    void setupPort(boost::asio::serial_port& serialPort, unsigned long baudRate)
    {
        serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
        serialPort.set_option(boost::asio::serial_port_base::character_size(8));
        serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    }
};

void AddBoostProgramOptions(SerialPortInformation& portInformation)
{
    using namespace boost::program_options;

    const char* HELP = "help";
    const char* PORT = "port";
    const char* BAUD_RATE = "baud_rate";

        options_description description("Options");
        description.add_options()
            (HELP, "show help message")
            (PORT, value<std::string>(&portInformation.portName)->default_value("/dev/pts/3"), "set serial port")
            (BAUD_RATE, value<unsigned long>(&portInformation.baudRate)->default_value(9600), "set baud rate")
        ;

        variables_map variableMap;
        store(parse_command_line(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv, description), variableMap);
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
}

void Setup_SerialServer_And_Check_Equal_With_SerialServer_Output(const char* testString, int numberOfChars)
{
    SerialPortInformation portInformation;
    AddBoostProgramOptions(portInformation);
    for (int i = 0; i < numberOfChars; ++i)
        portInformation.sendString.push_back(testString[i]);

    boost::asio::io_context io_context;
    SerialServer serialServer(io_context, portInformation);

    BOOST_CHECK_EQUAL(serialServer.getBufferData(), testString);
}

BOOST_AUTO_TEST_SUITE(test_suit)

BOOST_AUTO_TEST_CASE(test_less_than_buffer_size)
{
    Setup_SerialServer_And_Check_Equal_With_SerialServer_Output("test_conn ", 11);
}

BOOST_AUTO_TEST_CASE(test_more_than_buffer_size)
{
    Setup_SerialServer_And_Check_Equal_With_SerialServer_Output("test_connection ", 17);
}

BOOST_AUTO_TEST_CASE(test_non_ASCII_or_null)
{
    Setup_SerialServer_And_Check_Equal_With_SerialServer_Output("tēst_\0čo ", 12);
}

BOOST_AUTO_TEST_SUITE_END()
