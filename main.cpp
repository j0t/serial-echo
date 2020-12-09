#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

// Sync sertial echo device app 

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
        boost::asio::serial_port port(io_context, argv[1]);

        for (;;)
        {
            boost::array<char, 128> buffer;
            boost::system::error_code error;

            size_t lenght = port.read_some(boost::asio::buffer(buffer), error);

            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer
            else if (error)
                throw boost::system::system_error(error);

            std::cout.write(buffer.data(), lenght);

            port.write_some(boost::asio::buffer(buffer), error);

            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer
            else if (error)
                throw boost::system::system_error(error);
        }
        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}
