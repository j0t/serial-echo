#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    std::cout << "The port: " << argv[1] << std::endl;

    boost::asio::io_context io_context;

    // Create the parameter as an tcp object
    tcp::resolver resolver(io_context);
    // Create the endpoints to which connection is established
    tcp::resolver::results_type endpoints =
      resolver.resolve(argv[1], "daytime");
    
    // Create a socket
    tcp::socket socket(io_context);
    // Connect to a socket
    boost::asio::connect(socket, endpoints);

    // To hold the data, which is recieved
    for (;;)
    {
      boost::array<char, 128> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.

      std::cout.write(buf.data(), len);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
