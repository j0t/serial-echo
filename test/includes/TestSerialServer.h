#pragma once

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/io/ios_state.hpp>

#include "utility.h"
#include "SerialServer.h"
#include "SerialPortInformation.h"

class TestSerialServer : public SerialServer
{
private:
    boost::asio::streambuf dataBuffer;

public:
    TestSerialServer(boost::asio::io_context &io_context, SerialPortInformation &portInformation);

    void readData(char endChar, std::vector<char> &inputVector);
    void writeData(std::vector<char> &sendString);

    void getBufferData(std::vector<char> &inputVector);
};
