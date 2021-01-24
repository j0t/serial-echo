#define BOOST_TEST_TOOLS_UNDER_DEBUGGER
#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/bind/bind.hpp>
#include "TestSerialServer.h"
#include "Logger.h"

class TestSerialServerFixture
{
public:
    TestSerialServer serialServer;

public:
    TestSerialServerFixture(boost::asio::io_context& io_context, SerialPortInformation& portInformation)
        : serialServer(io_context, portInformation)
    {
        if (portInformation.debugLevel == 1)
            std::cout << "Fixture created!" << std::endl;
    }

    ~TestSerialServerFixture() = default;

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

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
    std::streambuf* const coutbuf = std::cout.rdbuf();
    std::clog.rdbuf(new Log("serial-echo-test", LOG_LOCAL0));
    
    teebuf* tee = new teebuf(coutbuf, std::clog.rdbuf());
    std::cout.rdbuf(tee);

    SerialPortInformation* portInformation = new SerialPortInformation(argc, argv);
    boost::asio::io_context* io_context = new boost::asio::io_context;

    TestSerialServerFixture* testServer = new TestSerialServerFixture(*io_context, *portInformation);

    const char* testStringsForEcho[] = {
        "test_conn!",
        "test_connection$",
        "tēst_\0čo#",
        "tēst_\x01čo@"
    };

    const char* testStringsForPairing[] = {
        "Send RTS1!",
        "Send RTS0!"
    };

    boost::function<void (const char*)> CompareEchoTest = bind( &TestSerialServerFixture::CompareEcho, testServer, boost::placeholders::_1);
    boost::function<void (const char*)> CTS_RTS_Pairing_Test = bind( &TestSerialServerFixture::Test_CTS_RTS_Pairing, testServer, boost::placeholders::_1);

    test_suite* ts1 = BOOST_TEST_SUITE("test_data_transfer");

    ts1->add(BOOST_PARAM_TEST_CASE( CompareEchoTest, testStringsForEcho, 
        testStringsForEcho + sizeof(testStringsForEcho) / sizeof(testStringsForEcho[0]) ));

    test_suite* ts2 = BOOST_TEST_SUITE("test_modem");
  
    ts2->add(BOOST_PARAM_TEST_CASE( CTS_RTS_Pairing_Test, testStringsForPairing, 
        testStringsForPairing + sizeof(testStringsForPairing) / sizeof(testStringsForPairing[0]) ));

    framework::master_test_suite().add( ts1 );
    framework::master_test_suite().add( ts2 );

    std::cout.rdbuf(coutbuf);
    return 0;
}
