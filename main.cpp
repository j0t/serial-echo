#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>


// Sync sertial echo device app 

class SerialConnection : public boost::enable_shared_from_this<SerialConnection>
{
private:
    boost::asio::serial_port serialPort;
    std::string message;

public:
    typedef boost::shared_ptr<SerialConnection> pointer;

    static pointer create(boost::asio::io_context& io_context, const char* portName)
    {
        return pointer(new SerialConnection(io_context, portName));
    }

    boost::asio::serial_port& getSerialPort()
    {
        return this->serialPort;
    }

    void start()
    {
        this->message = "Testing connection!\n";

        boost::asio::async_write(this->serialPort, boost::asio::buffer(this->message),
            boost::bind(&SerialConnection::handleWrite, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

private:
    SerialConnection(boost::asio::io_context& io_context, const char* portName)
        : serialPort(io_context, portName)
    {
        ;
    }

    void handleWrite(const boost::system::error_code&, size_t)
    {
        ;
    }
};

class SerialServer
{
private:
    boost::asio::io_context& io_context;
    const char* portName;

public:
    SerialServer(boost::asio::io_context& io_context, const char* portName)
        : io_context(io_context)
        , portName(portName)
    {
        start_accept();
    }

private:
    void start_accept()
    {
        SerialConnection::pointer newConnection = 
            SerialConnection::create(this->io_context, this->portName);
    }

    void handleAccept(SerialConnection::pointer newConnection, 
        const boost::system::error_code& error)
    {
        if (!error)
        {
            newConnection->start();
        }

        start_accept();
    }

};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: ./sertial-echo <port> (ex. /dev/ttyS0)";
            return 1;
        }

        boost::asio::io_context io_context;
        SerialServer serialPort(io_context, argv[1]);
        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
