#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

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
    std::string message;

public:
    SerialServer(boost::asio::io_context& io_context, SerialPortInformation& portInformation)
        : io_context(io_context)
        , portInformation(portInformation)
        , serialPort(io_context, portInformation.portName)
    {
        setupPort(this->serialPort, this->portInformation.baudRate);
        startActions();
    }

public:
    void startActions()
    {
        // Implement read here

        this->message = "Testing connection!\n";

        std::cout << "Writing message: " << this->message << std::endl;

        boost::asio::async_write(this->serialPort, boost::asio::buffer(this->message),
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

    void handleWrite(const boost::system::error_code&, size_t)
    {
        ;
    }
};

int main(int argc, const char* argv[])
{
    using namespace boost::program_options;

    try
    {  
        SerialPortInformation portInformation;

        options_description description("Options");
        description.add_options()
            ("help", "show help message")
            ("port", value<std::string>(&portInformation.portName)->default_value("/dev/ttyS0"), "set serial port")
            ("baud_rate", value<unsigned long>(&portInformation.baudRate)->default_value(9600), "set baud rate")
        ;

        variables_map variableMap;
        store(parse_command_line(argc, argv, description), variableMap);
        notify(variableMap);

        if (variableMap.count("help"))
        {
            std::cout << description << "\n";
            return 0;
        }
        
        if (variableMap.count("port"))
        {
            std::cout << "Serial port device was set to " << variableMap["port"].as<std::string>() << "\n";
        }
        else
        {
            std::cout << "Serial port device was set to default\n";
        }

        if (variableMap.count("baud_rate"))
        {
            std::cout << "Serial port device baud rate was set to " << variableMap["baud_rate"].as<unsigned long>() << "\n";
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
