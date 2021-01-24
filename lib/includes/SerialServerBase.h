#pragma once

#include <boost/asio.hpp>
#include "SerialPortInformation.h"

class SerialServerBase
{
protected:
    SerialPortInformation& portInformation;
    boost::asio::serial_port serialPort;

    int fd;
    unsigned int modemStatus = 0;
    unsigned int oldModemStatus = 0;

public:
    SerialServerBase(boost::asio::io_context& io_context, SerialPortInformation& portInformation);
    virtual ~SerialServerBase();

public:
    void setupPort(boost::asio::serial_port& serialPort, unsigned long baudRate);

    void setModemStatus(unsigned int signal, bool value);
    int getModemSignals();
    virtual void manageRTS() {}

    boost::asio::serial_port getSerialPort() const;
};
