#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

static const unsigned int BUFFER_SIZE = 12;

struct SerialPortInformation
{
    std::string portName;
    unsigned long baudRate;
};

class SerialServer
{
private:
    boost::asio::io_context& io_context;
    SerialPortInformation& portInformation;

    boost::asio::serial_port serialPort;
    boost::array<char, BUFFER_SIZE> dataBuffer;

    bool readComplete = false;

public:
    SerialServer(boost::asio::io_context& io_context, SerialPortInformation& portInformation)
        : io_context(io_context)
        , portInformation(portInformation)
        , serialPort(io_context, portInformation.portName)
    {
        setupPort(this->serialPort, this->portInformation.baudRate);
        startRead();
        if (this->readComplete)
            startWrite();
    }

public:
    void startRead()
    {
        boost::asio::async_read(this->serialPort, boost::asio::buffer(this->dataBuffer, BUFFER_SIZE),
            boost::bind(&SerialServer::handleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    void startWrite()
    {
        boost::asio::async_write(this->serialPort, boost::asio::buffer(this->dataBuffer, BUFFER_SIZE),
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
    }

    void handleRead(const boost::system::error_code& error, size_t length)
    {
        if (!error)
        {
            std::cout << "Read message: ";
            std::cout.write(this->dataBuffer.data(), length);
            std::cout << " | Recieved length: " << length << "\n";
        }
        else
            std::cerr << "Handle read! | " << "Error: " << error << " | Data length: " << length << "\n";

        this->readComplete = true;
    }

    void handleWrite(const boost::system::error_code& error, size_t length)
    {
        if (!error)
        {
            std::cout << "Write message: " << std::hex << std::uppercase << this->dataBuffer.data() << std::dec << " | Recieved length: " << length << "\n";
        }
        else
            std::cerr << "Handle write! | " << "Error: " << error << " | Data length: " << length << "\n";
    }
};

int main(int argc, const char* argv[])
{
    using namespace boost::program_options;

    const char* HELP = "help";
    const char* PORT = "port";
    const char* BAUD_RATE = "baud_rate";

    try
    {  
        SerialPortInformation portInformation;

        options_description description("Options");
        description.add_options()
            (HELP, "show help message")
            (PORT, value<std::string>(&portInformation.portName)->default_value("/dev/ttyS0"), "set serial port")
            (BAUD_RATE, value<unsigned long>(&portInformation.baudRate)->default_value(9600), "set baud rate")
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

        std::cout << "Opening port: " << portInformation.portName << std::endl;

        boost::asio::io_context io_context;
        SerialServer serialPort(io_context, portInformation);
        io_context.run();

        std::cout << "Closing port\n";

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
