#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

// Serverside tcp app, which shows the daytime information on the socket
// try - netcat 127.0.0.53 13
// to see - Tue Dec  8 13:12:02 2020

using boost::asio::ip::tcp;

// The string to be put into the port
std::string make_daytime_string()
{
    std::time_t now = time(0);
    return std::ctime(&now);
}

int main()
{
    try
    {
        boost::asio::io_context io_context;
        
        // listen on port 13 with ip version 4
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));

        // Iterative server which is going to accept a single connection at a time
        for (;;)
        {   
            // Connection to the client
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            // Client is accessing the service
            std::string message = make_daytime_string();

            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
