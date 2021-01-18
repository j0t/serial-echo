#include "TestSerialServer.h"

TestSerialServer::TestSerialServer(boost::asio::io_context &io_context, SerialPortInformation &portInformation)
    : SerialServer(io_context, portInformation)
{
    setupPort(this->serialPort, this->portInformation.baudRate);
}

void TestSerialServer::readData(char endChar, std::vector<char> &inputVector)
{
    boost::system::error_code error;
    boost::asio::read_until(this->serialPort, this->dataBuffer, endChar, error);
    getBufferData(inputVector);
}

void TestSerialServer::writeData(std::vector<char> &sendString)
{
    boost::system::error_code error;
    boost::asio::write(this->serialPort, boost::asio::buffer(sendString, sendString.size()), error);
}

void TestSerialServer::getBufferData(std::vector<char> &inputVector)
{
    std::istream inputStream(&this->dataBuffer);
    inputVector.assign(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>());
}
