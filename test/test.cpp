// #define BOOST_TEST_TOOLS_UNDER_DEBUGGER

#define BOOST_TEST_MODULE
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API

#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/bind/bind.hpp>
#include "TestSerialServer.h"
#include "Logger.h"

class TestSerialServerFixture
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
        if (this->portInformation.debugLevel == 1)
            std::cout << "Fixture created!" << std::endl;
    }

    ~TestSerialServerFixture()
    {
        if (this->portInformation.debugLevel == 1)
            std::cout << "Fixture destroyed!" << std::endl;
    }

    void CompareEcho(const char *testString)
    {
        std::cout << "CompareEcho test" << std::endl;

        std::vector<char> bufferData, sendString, testDataVector;

        unsigned int testStringLenght = strlen(testString) + 1;
        char endChar = testString[strlen(testString) - 1];

        sendString.assign(testString, testString + testStringLenght);

        this->serialServer.writeData(sendString);
        this->serialServer.readData(endChar, bufferData);

        makeVector(testDataVector, testString, testStringLenght);

        BOOST_CHECK_EQUAL_COLLECTIONS(bufferData.begin(), bufferData.end(), testDataVector.begin(), testDataVector.end());
    }

    void Test_CTS_RTS_Pairing(const char *testString)
    {
        std::cout << "Test_CTS_RTS_Pairing test" << std::endl;

        std::vector<char> bufferData, sendString, testDataVector;

        int modemSignals = 0;
        unsigned int testStringLenght = strlen(testString) + 1;
        char endChar = testString[strlen(testString) - 1];

        sendString.assign(testString, testString + testStringLenght);
        makeVector(testDataVector, testString, testStringLenght);

        this->serialServer.manageRTS();

        this->serialServer.writeData(sendString);

        this->serialServer.readData(endChar, bufferData);
        
        modemSignals = this->serialServer.getModemSignals();

        BOOST_CHECK_EQUAL(modemSignals & TIOCM_CTS, TIOCM_CTS);

        BOOST_CHECK_EQUAL_COLLECTIONS(bufferData.begin(), bufferData.end(), testDataVector.begin(), testDataVector.end());
    }
};

BOOST_FIXTURE_TEST_SUITE(test_data_transfer, TestSerialServerFixture)

BOOST_AUTO_TEST_CASE(test_less_than_buffer_size)
{
    CompareEcho("test_conn!");
}

BOOST_AUTO_TEST_CASE(test_more_than_buffer_size)
{
    CompareEcho("test_connection$");
}

BOOST_AUTO_TEST_CASE(test_null)
{
    CompareEcho("tēst_\0čo#");
}

BOOST_AUTO_TEST_CASE(test_non_ASCII)
{
    CompareEcho("tēst_\x01čo@");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(test_modem, TestSerialServerFixture)

BOOST_AUTO_TEST_CASE(test_CTS)
{
    Test_CTS_RTS_Pairing("Send RTS1!");
    std::cout << "Test 1 done!" << std::endl;
    
    Test_CTS_RTS_Pairing("Send RTS0!");
    std::cout << "Test 2 done!" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

void make_use_of(char**)
{
  std::cout << "Using custom entry point..." << std::endl;
}

int main(int argc, char* argv[], char* envp[])
{
    std::streambuf * const coutbuf = std::cout.rdbuf();
    std::clog.rdbuf(new Log("serial-echo-test", LOG_LOCAL0));

    teebuf tee(coutbuf, std::clog.rdbuf());
    std::cout.rdbuf(&tee);

    make_use_of(envp);

    std::cout.rdbuf(coutbuf);
    return boost::unit_test::unit_test_main(init_unit_test, argc, argv);
}
