#pragma once

#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/io/ios_state.hpp>
#include "utility.h"
#include "SerialServerBase.h"

static const unsigned int BUFFER_SIZE = 12;

class AsyncSerialServer : public SerialServerBase
{
private:
    boost::array<char, BUFFER_SIZE> dataBuffer;

public:
    AsyncSerialServer(boost::asio::io_context& io_context, SerialPortInformation& portInformation);
    ~AsyncSerialServer();

public:
    void startRead();
    void startWrite(size_t length);

    void manageRTS() override;

    void handleRead(const boost::system::error_code& error, size_t length);
    void handleWrite(const boost::system::error_code& error, size_t length);

    void printInformation(const char* messageType, const boost::system::error_code& error, size_t length);
};
