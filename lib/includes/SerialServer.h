#pragma once

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/io/ios_state.hpp>

#include "SerialPortInformation.h"
#include "utility.h"

static const unsigned int BUFFER_SIZE = 12;

class SerialServer
{
protected:
    SerialPortInformation& portInformation;

    boost::asio::serial_port serialPort;
    boost::array<char, BUFFER_SIZE> dataBuffer;

    int fd;
    int modemStatus = 0;
    int oldModemStatus = 0;

public:
    SerialServer(boost::asio::io_context& io_context, SerialPortInformation& portInformation);
    ~SerialServer();

public:
    void startRead();
    void startWrite(size_t length);

    void setupPort(boost::asio::serial_port& serialPort, unsigned long baudRate);

    void setModemStatus(unsigned int signal, bool value);
    int getModemSignals();
    virtual void manageRTS();

    void handleRead(const boost::system::error_code& error, size_t length);
    void handleWrite(const boost::system::error_code& error, size_t length);

    void printInformation(const char* messageType, const boost::system::error_code& error, size_t length);
};
