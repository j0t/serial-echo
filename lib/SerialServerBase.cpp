#include "SerialServerBase.h"

SerialServerBase::SerialServerBase(boost::asio::io_context& io_context, SerialPortInformation& portInformation)
    : portInformation(portInformation)
    , serialPort(io_context, portInformation.portName)
{
    setupPort(this->serialPort, this->portInformation.baudRate);
}

SerialServerBase::~SerialServerBase()
{
}

void SerialServerBase::setupPort(boost::asio::serial_port& serialPort, unsigned long baudRate)
{
    serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
    serialPort.set_option(boost::asio::serial_port_base::character_size(8));
    serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
    serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    this->fd = serialPort.native_handle();
}

void SerialServerBase::setModemStatus(unsigned int signal, bool value)
{
    int returnCode = ioctl(this->fd, value ? TIOCMBIS : TIOCMBIC, &signal);

    std::string signalType = modemStatusToString(signal);

    if (returnCode < 0)
        throw boost::system::system_error(returnCode, boost::system::system_category(), (signalType + " couldn\'t be set/cleared"));

    if (this->portInformation.debugLevel == 1)
        std::cout << (value ? (signalType + " set!") : (signalType + " cleared!")) << std::endl;
}

int SerialServerBase::getModemSignals()
{
    int modemData = 0;
    int returnCode = ioctl(this->fd, TIOCMGET, &modemData);
    
    if (returnCode < 0)
        throw boost::system::system_error(returnCode, boost::system::system_category(), "Failed to TIOCMGET");

    if (this->portInformation.debugLevel == 1)
        std::cout << "ModemData: " << std::hex << (modemData) << std::dec << modemDataTypesToString(modemData) << std::endl;

    return modemData;
}
