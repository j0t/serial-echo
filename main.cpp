#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

// Sync sertial echo device app 

struct SerialPortInformation
{
    std::string portName;
    unsigned long baudRate;
};

class SerialServer : public boost::enable_shared_from_this<SerialServer>
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
            boost::bind(&SerialServer::handleWrite, shared_from_this(),
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

int main()
{
    try
    {
        SerialPortInformation portInformation;

        std::cout << "Enter serial port: ";
        std::cin >> portInformation.portName;

        std::cout << "Enter baud rate: ";
        std::cin >> portInformation.baudRate;

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
