#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <sys/ioctl.h>

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
    unsigned int debugLevel;

public:
    SerialPortInformation(int argc, char *argv[])
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
    }
};

class SerialServer
{
private:
    boost::asio::io_context &io_context;
    SerialPortInformation portInformation;

    boost::asio::serial_port serialPort;
    boost::asio::streambuf dataBuffer;

    int fd;
    int modemStatus = 0;
    int oldModemStatus = 0;

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
        this->fd = serialPort.native_handle();
    }

    void setRTS(int RTSvalue)
    {
        int modemData = TIOCM_RTS;
        int returnCode = ioctl(this->fd, RTSvalue != 0 ? TIOCMBIS : TIOCMBIC, &modemData);

        if (returnCode < 0)
            throw boost::system::system_error(returnCode, boost::system::system_category(), "RTS couldn\'t be cleared");

        if (this->portInformation.debugLevel == 1)
            std::cout << "RTS cleared!\n";
    }

    void setDTR(int DTRvalue)
    {
        int modemData = TIOCM_DTR;
        int returnCode = ioctl(this->fd, DTRvalue != 0 ? TIOCMBIS : TIOCMBIC, &modemData);

        if (returnCode < 0)
            throw boost::system::system_error(returnCode, boost::system::system_category(), "DTR couldn\'t be cleared");
        
        if (this->portInformation.debugLevel == 1)
            std::cout << "DTR cleared!\n";
    }

    int getModemSignals()
    {
        int modemData = 0;
        int returnCode = ioctl(this->fd, TIOCMGET, &modemData);
        
        if (returnCode < 0)
            throw boost::system::system_error(returnCode, boost::system::system_category(), "Failed to TIOCMGET");
        
        if (this->portInformation.debugLevel == 1)
            std::cout << "ModemData: " << std::hex << modemData << std::dec << "\n";

        return modemData;
    }

    void manageRTS()
    {
        this->modemStatus = getModemSignals();
        
        if (this->modemStatus != 0 && (this->oldModemStatus& TIOCM_CTS) != (this->modemStatus& TIOCM_CTS))
        {   
            this->oldModemStatus = this->modemStatus;
            setRTS(this->modemStatus& TIOCM_CTS);
        }
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

    void Test_CTR_RTS_Pairing()
    {
        std::vector<char> bufferData, sendString, testDataVector;

        const char* testString = "Send RTS!";
        int modemSignals = 0;

        sendString.assign(testString, testString + 10);
        makeVector(testDataVector, testString, 10);

        this->serialServer.manageRTS();
        this->serialServer.writeData(sendString);
        modemSignals = this->serialServer.getModemSignals();
        
        BOOST_CHECK_EQUAL(modemSignals & TIOCM_CTS, TIOCM_CTS);
        BOOST_CHECK(modemSignals & TIOCM_CTS);
        BOOST_CHECK_EQUAL(modemSignals, TIOCM_CTS);
        
        this->serialServer.readData('!', bufferData);

        BOOST_CHECK_EQUAL_COLLECTIONS(bufferData.begin(), bufferData.end(), testDataVector.begin(), testDataVector.end());
    }
};

BOOST_FIXTURE_TEST_SUITE(test_data_transfer, TestSerialServerFixture)

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

BOOST_FIXTURE_TEST_SUITE(test_modem, TestSerialServerFixture)

BOOST_AUTO_TEST_CASE(test_CTS)
{
    Test_CTR_RTS_Pairing();
}

BOOST_AUTO_TEST_SUITE_END()
