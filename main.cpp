#include <iostream>
#include <sys/ioctl.h>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <boost/io/ios_state.hpp>

static const unsigned int BUFFER_SIZE = 12;

struct SerialPortInformation
{
    std::string portName;
    unsigned long baudRate;
    unsigned int debugLevel;
};

class SerialServer
{
private:
    boost::asio::io_context& io_context;
    SerialPortInformation& portInformation;

    boost::asio::serial_port serialPort;
    boost::array<char, BUFFER_SIZE> dataBuffer;

    int fd;
    int modemStatus = 0;
    int oldModemStatus = 0;

public:
    SerialServer(boost::asio::io_context& io_context, SerialPortInformation& portInformation)
        : io_context(io_context)
        , portInformation(portInformation)
        , serialPort(io_context, portInformation.portName)
    {
        setupPort(this->serialPort, this->portInformation.baudRate);
        startRead();
    }

public:
    void startRead()
    {
        manageRTS();

        this->serialPort.async_read_some(boost::asio::buffer(this->dataBuffer, BUFFER_SIZE),
            boost::bind(&SerialServer::handleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    void startWrite(size_t length)
    {
        manageRTS();

        boost::asio::async_write(this->serialPort, boost::asio::buffer(this->dataBuffer, length),
            boost::bind(&SerialServer::handleWrite, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    void setupPort(boost::asio::serial_port& serialPort, unsigned long baudRate)
    {
        serialPort.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
        serialPort.set_option(boost::asio::serial_port_base::character_size(8));
        serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        this->fd = serialPort.native_handle();
    }

    void setModemStatus(int signal, bool value)
    {
        int returnCode = ioctl(this->fd, value ? TIOCMBIS : TIOCMBIC, &signal);

        std::string signalType;
        if ((signal & TIOCM_RTS) && (signal & TIOCM_DTR))
            signalType = "RTS/DTS";
        else if (signal& TIOCM_RTS)
            signalType = "RTS";
        else
            signalType = "DTR";

        if (returnCode < 0)
            throw boost::system::system_error(returnCode, boost::system::system_category(), (signalType + " couldn\'t be set/cleared"));

        if (this->portInformation.debugLevel == 1)
            std::cout << (value ? (signalType, " set!\n") : (signalType, " cleared!\n"));
    }

    int getModemSignals()
    {
        int modemData = 0;
        int returnCode = ioctl(this->fd, TIOCMGET, &modemData);
        
        if (returnCode < 0)
            throw boost::system::system_error(returnCode, boost::system::system_category(), "Failed to TIOCMGET");
        
        if (this->portInformation.debugLevel == 1)
            std::cout << "ModemData: " << std::hex << modemData << std::dec << "\n";

        return modemData;
    }

    void manageRTS()
    {
        this->modemStatus = getModemSignals();
        
        if (this->modemStatus != 0 && (this->oldModemStatus& TIOCM_CTS) != (this->modemStatus& TIOCM_CTS))
        {   
            this->oldModemStatus = this->modemStatus;
            setModemStatus(TIOCM_RTS, this->modemStatus& TIOCM_CTS);
        }
    }

    void handleRead(const boost::system::error_code& error, size_t length)
    {
        if (this->portInformation.debugLevel == 1)
            printInformation("Read", error, length);

        startWrite(length);
    }

    void handleWrite(const boost::system::error_code& error, size_t length)
    {
        if (this->portInformation.debugLevel == 1)
            printInformation("Write", error, length);

        startRead();
    }

    void printInformation(const char* messageType, const boost::system::error_code& error, size_t length)
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

            std::cout << " | Message length: " << length << "\n";
        }
        else
        {
            std::cerr << "[Error]: Handle " << messageType << "! | " << "Error: " << error << 
                " | modemData length: " << length << " - Must be " << BUFFER_SIZE << " bytes!" << "\n";
            throw error;
        }
    }
};

int main(int argc, const char* argv[])
{
    using namespace boost::program_options;

    const char* HELP = "help";
    const char* PORT = "port";
    const char* BAUD_RATE = "baud_rate";
    const char* DEBUG_LEVEL = "debug_level";

    try
    {  
        SerialPortInformation portInformation;

        options_description description("Options");
        description.add_options()
            (HELP, "show help message")
            (PORT, value<std::string>(&portInformation.portName)->default_value("/dev/ttyS0"), "set serial port")
            (BAUD_RATE, value<unsigned long>(&portInformation.baudRate)->default_value(9600), "set baud rate")
            (DEBUG_LEVEL, value<unsigned int>(&portInformation.debugLevel)->default_value(0), "set debug level (0 - none, 1 - full)")
        ;

        variables_map variableMap;
        store(parse_command_line(argc, argv, description), variableMap);
        notify(variableMap);

        if (variableMap.count(HELP))
        {
            std::cout << description << "\n";
            return 0;
        }
        
        if (variableMap.count(PORT))
        {
            std::cout << "Serial port device was set to " << variableMap[PORT].as<std::string>() << "\n";
        }
        else
        {
            std::cout << "Serial port device was set to default\n";
        }

        if (variableMap.count(BAUD_RATE))
        {
            std::cout << "Serial port device baud rate was set to " << variableMap[BAUD_RATE].as<unsigned long>() << "\n";
        }
        else
        {
            std::cout << "Serial device baud rate was set to default\n";
        }

        if (variableMap.count(DEBUG_LEVEL))
        {
            std::cout << "Debug level was set to " << variableMap[DEBUG_LEVEL].as<unsigned int>() << "\n";
        }
        else
        {
            std::cout << "Debug level was set to default\n";
        }

        std::cout << "Opening port: " << portInformation.portName << std::endl;

        boost::asio::io_context io_context;
        SerialServer serialPort(io_context, portInformation);
        io_context.run();

        std::cout << "Closing port\n";

    }
    catch(const boost::system::system_error& e)
    {
        std::cerr << "[ERROR]: " << e.what() << ": " << e.code() << " - " << e.code().message() << '\n';
    }
    
    return 0;
}
