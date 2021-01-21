#include "SerialServer.h"

SerialServer::SerialServer(boost::asio::io_context& io_context, SerialPortInformation& portInformation)
    : portInformation(portInformation)
    , serialPort(io_context, portInformation.portName)
{
    setupPort(this->serialPort, this->portInformation.baudRate);
    startRead();
}

SerialServer::~SerialServer()
{
}

void SerialServer::startRead()
{
    manageRTS();

    this->serialPort.async_read_some(boost::asio::buffer(this->dataBuffer, BUFFER_SIZE),
        boost::bind(&SerialServer::handleRead, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void SerialServer::startWrite(size_t length)
{
    manageRTS();

    boost::asio::async_write(this->serialPort, boost::asio::buffer(this->dataBuffer, length),
        boost::bind(&SerialServer::handleWrite, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void SerialServer::setupPort(boost::asio::serial_port& serialPort, unsigned long baudRate)
{
    serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
    serialPort.set_option(boost::asio::serial_port_base::character_size(8));
    serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
    serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    this->fd = serialPort.native_handle();
}

void SerialServer::setModemStatus(unsigned int signal, bool value)
{
    int returnCode = ioctl(this->fd, value ? TIOCMBIS : TIOCMBIC, &signal);

    std::string signalType = modemStatusToString(signal);

    if (returnCode < 0)
        throw boost::system::system_error(returnCode, boost::system::system_category(), (signalType + " couldn\'t be set/cleared"));

    if (this->portInformation.debugLevel == 1)
        std::cout << (value ? (signalType + " set!") : (signalType + " cleared!")) << std::endl;
}

int SerialServer::getModemSignals()
{
    int modemData = 0;
    int returnCode = ioctl(this->fd, TIOCMGET, &modemData);
    
    if (returnCode < 0)
        throw boost::system::system_error(returnCode, boost::system::system_category(), "Failed to TIOCMGET");

    if (this->portInformation.debugLevel == 1)
        std::cout << "ModemData: " << std::hex << (modemData) << std::dec << modemDataTypesToString(modemData) << std::endl;

    return modemData;
}

void SerialServer::manageRTS()
{
    this->modemStatus = getModemSignals();
    
    if (this->modemStatus != 0)
    {   
        this->oldModemStatus = this->modemStatus;
        setModemStatus(TIOCM_RTS, this->modemStatus & TIOCM_CTS);
    }
    else
        if (this->portInformation.debugLevel == 1)
            std::cout << "Skipped RTS" << std::endl;
}

void SerialServer::handleRead(const boost::system::error_code& error, size_t length)
{
    if (this->portInformation.debugLevel == 1)
        printInformation("Read", error, length);

    startWrite(length);
}

void SerialServer::handleWrite(const boost::system::error_code& error, size_t length)
{
    if (this->portInformation.debugLevel == 1)
        printInformation("Write", error, length);

    startRead();
}

void SerialServer::printInformation(const char* messageType, const boost::system::error_code& error, size_t length)
{
    if (!error)
    {
        boost::io::ios_all_saver  ias( std::cout );

        std::cout << messageType << " message: ";
        for (size_t i = 0; i < length; i++)
        {
            if(isprint(this->dataBuffer[i]))
                std::cout << this->dataBuffer[i];
            else
            {
                auto ch = static_cast<int>(0xFF&this->dataBuffer[i]);
                switch (ch)
                {
                case 0:
                    std::cout << "[\\0]";
                    break;
                
                case 10:
                    std::cout << "[\\n]";
                    break;

                case 13:
                    std::cout << "[\\r]";
                    break;
                
                case 9:
                    std::cout << "[\\t]";
                    break;
                
                case 11:
                    std::cout << "[\\v]";
                    break;

                case 8:
                    std::cout << "[\\b]";
                    break;
                
                case 12:
                    std::cout << "[\\f]";
                    break;
                
                case 7:
                    std::cout << "[\\a]";
                    break;

                default:
                    std::cout << "[" << std::hex << std::uppercase << ch << "]";
                    break;
                }
            }
        }

        ias.restore();

        std::cout << " | Message length: " << length << std::endl;
    }
    else
    {
        std::cerr << "[Error]: Handle " << messageType << "! | " << "Error: " << error << 
            " | modemData length: " << length << " - Must be " << BUFFER_SIZE << " bytes!" << std::endl;
        throw error;
    }
}
