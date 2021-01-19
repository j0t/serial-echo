#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "Logger.h"
#include "utility.h"
#include "TestSerialServer.h"
#include "SerialPortInformation.h"

struct TestSerialServerFixture
{
public:
    SerialPortInformation portInformation;
    boost::asio::io_context io_context;
    TestSerialServer serialServer;

public:
    TestSerialServerFixture()
        : portInformation(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv)
        , serialServer(this->io_context, this->portInformation)
    {
        std::clog.rdbuf(new Log("serial-echo-test", LOG_LOCAL0));
    }

    ~TestSerialServerFixture() = default;

    void CompareEcho(const char *testString, char endChar)
    {
        std::vector<char> bufferData, sendString, testDataVector;

        unsigned int testStringLenght = strlen(testString) + 1;

        sendString.assign(testString, testString + testStringLenght);

        this->serialServer.writeData(sendString);
        this->serialServer.readData(endChar, bufferData);

        makeVector(testDataVector, testString, testStringLenght);

        BOOST_CHECK_EQUAL_COLLECTIONS(bufferData.begin(), bufferData.end(), testDataVector.begin(), testDataVector.end());
    }

    void Test_CTS_RTS_Pairing(const char *testString, char endChar)
    {
        std::vector<char> bufferData, sendString, testDataVector;

        int modemSignals = 0;
        unsigned int testStringLenght = strlen(testString) + 1;

        sendString.assign(testString, testString + testStringLenght);
        makeVector(testDataVector, testString, testStringLenght);

        this->serialServer.manageRTS();

        sleep(2);

        this->serialServer.writeData(sendString);

        sleep(2);

        this->serialServer.readData(endChar, bufferData);
        
        modemSignals = this->serialServer.getModemSignals();

        sleep(2);

        BOOST_CHECK_EQUAL(modemSignals & TIOCM_CTS, TIOCM_CTS);

        BOOST_CHECK_EQUAL_COLLECTIONS(bufferData.begin(), bufferData.end(), testDataVector.begin(), testDataVector.end());
    }
};

BOOST_FIXTURE_TEST_SUITE(test_data_transfer, TestSerialServerFixture)

BOOST_AUTO_TEST_CASE(test_less_than_buffer_size)
{
    CompareEcho("test_conn!", '!');
}

BOOST_AUTO_TEST_CASE(test_more_than_buffer_size)
{
    CompareEcho("test_connection$", '$');
}

BOOST_AUTO_TEST_CASE(test_null)
{
    CompareEcho("tēst_\0čo#", '#');
}

BOOST_AUTO_TEST_CASE(test_non_ASCII)
{
    CompareEcho("tēst_\x01čo@", '@');
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(test_modem, TestSerialServerFixture)

BOOST_AUTO_TEST_CASE(test_CTS)
{
    Test_CTS_RTS_Pairing("Send RTS1!", '!');
    
    std::clog << kLogDebug << "Test 1 done!" << std::endl;
    sleep(2);
    
    Test_CTS_RTS_Pairing("Send RTS0!", '!');
        
    std::clog << kLogDebug << "Test 2 done!" << std::endl;
    sleep(2);
}

BOOST_AUTO_TEST_SUITE_END()
