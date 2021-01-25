#define BOOST_TEST_MODULE

#include <boost/test/included/unit_test.hpp>
#include "TestSerialServer.h"
#include "Logger.h"

struct TestSerialServerFixture
{
public:
    boost::asio::io_context io_context;

    SerialPortInformation portInformation;
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

struct AllocateBuffers
{
    std::streambuf* const coutbuf = std::cout.rdbuf();
    
    AllocateBuffers()
    {
        std::clog.rdbuf(new Log("serial-echo-test", LOG_LOCAL0));
        teebuf* tee = new teebuf(coutbuf, std::clog.rdbuf());

        std::cout.rdbuf(tee);
    }
    ~AllocateBuffers()
    {
        std::cout.rdbuf(coutbuf);
    }
};

BOOST_GLOBAL_FIXTURE(AllocateBuffers);

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
    CompareEcho("tēst_\x01č@");
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

int main(int argc, char* argv[])
{
    Log syslog("serial-echo-test", LOG_LOCAL0);

    // attach std::clog to syslog
    Attach_rdbuf attach( std::clog, &syslog );

    // output to clog will appear in syslog
    // output to cout will appear in stdout and syslog
    Tee tee( std::cout, std::clog );

    return boost::unit_test::unit_test_main(init_unit_test, argc, argv);
}
