#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

// Serverside async tcp app, which shows the daytime information on the socket

using boost::asio::ip::tcp;

std::string make_daytime_string()
{
    std::time_t now = time(0);
    return std::ctime(&now);
}

// Uses a shared_ptr to keep the object in memory because it established the connection
// to the socket
class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
{
private:
    tcp::socket socket;
    std::string message;

private:
    tcp_connection(boost::asio::io_context& io_context)
        : socket(io_context)
    {
        ;
    }

    // Could be an method with no parameters, but leave them here for now
    void handle_write(const boost::system::error_code&, size_t)
    {
        ;
    }

public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& getSocket()
    {
        return this->socket;
    }

    // Write the data to the client
    void start()
    {
        this->message = make_daytime_string();

        // takes in a socket and message
        // error and bytes_transferred are not called in handle_write so
        // it could be removed
        boost::asio::async_write(this->socket, boost::asio::buffer(this->message),
            boost::bind(&tcp_connection::handle_write, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
};

class tcp_server
{
private:
    boost::asio::io_context& io_context;
    tcp::acceptor acceptor;

private:
    // create socket and initilize an aysnc accept operation to wait for new connection
    void start_accept()
    {
        tcp_connection::pointer new_connection = tcp_connection::create(this->io_context);

        // handle accept is called here when the accept operation is done
        this->acceptor.async_accept(new_connection->getSocket(),
            boost::bind(&tcp_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
    }

    // serviced the client request and class start_accept again to wait for new request
    void handle_accept(tcp_connection::pointer new_connection,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            new_connection->start();
        }

        start_accept();
    }

public:
    // Initilize an acceptor for tcp port 13
    tcp_server(boost::asio::io_context& io_context)
        : io_context(io_context)
        , acceptor(io_context, tcp::endpoint(tcp::v4(), 13))
    {
        start_accept();
    }
};

int main()
{
    try
    {
        boost::asio::io_context io_context;
        tcp_server server(io_context);
        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;    
}
