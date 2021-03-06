#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

void makeVector(std::vector<char> &inputVector, const char *data, std::size_t size = -1)
{
    std::vector<char> createVector;
    inputVector.assign(data, data + size);
}

struct SerialPortInformation
{
public:
    std::string portName;
    unsigned long baudRate;

public:
    SerialPortInformation(int argc, char *argv[])
    {
        using namespace boost::program_options;

        const char *HELP = "help";
        const char *PORT = "port";
        const char *BAUD_RATE = "baud_rate";

        options_description description("Options");
        description.add_options()(HELP, "show help message")(PORT, value<std::string>(&this->portName)->default_value("/dev/pts/3"), "set serial port")(BAUD_RATE, value<unsigned long>(&this->baudRate)->default_value(9600), "set baud rate");

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
    }
};

class SerialServer
{
private:
    boost::asio::io_context &io_context;
    SerialPortInformation portInformation;

    boost::asio::serial_port serialPort;
    boost::asio::streambuf dataBuffer;

public:
    SerialServer(boost::asio::io_context &io_context, SerialPortInformation &portInformation)
        : io_context(io_context)
        , portInformation(portInformation)
        , serialPort(io_context, portInformation.portName)
    {
        setupPort(this->serialPort, this->portInformation.baudRate);
    }

    void readData(char endChar, std::vector<char> &inputVector)
    {
        boost::system::error_code error;
        boost::asio::read_until(this->serialPort, this->dataBuffer, endChar, error);
        getBufferData(inputVector);
    }

    void writeData(std::vector<char> &sendString)
    {
        boost::system::error_code error;
        boost::asio::write(this->serialPort, boost::asio::buffer(sendString, sendString.size()), error);
    }

    void getBufferData(std::vector<char> &inputVector)
    {
        std::istream inputStream(&this->dataBuffer);
        inputVector.assign(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>());
    }

    void setupPort(boost::asio::serial_port &serialPort, unsigned long baudRate)
    {
        serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
        serialPort.set_option(boost::asio::serial_port_base::character_size(8));
        serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    }
};

struct TestSerialServerFixture
{
public:
    SerialPortInformation portInformation;
    boost::asio::io_context io_context;
    SerialServer serialServer;

public:
    TestSerialServerFixture()
        : portInformation(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv)
        , serialServer(this->io_context, this->portInformation)
    {
    }

    ~TestSerialServerFixture() = default;

    void CompareEcho(const char *testString, int numberOfChars, char endChar)
    {
        std::vector<char> bufferData, sendString, testDataVector;

        sendString.assign(testString, testString + numberOfChars);

        this->serialServer.writeData(sendString);
        this->serialServer.readData(endChar, bufferData);

        makeVector(testDataVector, testString, numberOfChars);

        BOOST_CHECK_EQUAL_COLLECTIONS(bufferData.begin(), bufferData.end(), testDataVector.begin(), testDataVector.end());
    }
};

BOOST_FIXTURE_TEST_SUITE(test_suit, TestSerialServerFixture)

BOOST_AUTO_TEST_CASE(test_less_than_buffer_size)
{
    CompareEcho("test_conn!", 11, '!');
}

BOOST_AUTO_TEST_CASE(test_more_than_buffer_size)
{
    CompareEcho("test_connection$", 17, '$');
}

BOOST_AUTO_TEST_CASE(test_null)
{
    CompareEcho("tēst_\0čo#", 12, '#');
}

BOOST_AUTO_TEST_CASE(test_non_ASCII)
{
    CompareEcho("tēst_\x01čo@", 12, '@');
}

BOOST_AUTO_TEST_SUITE_END()
