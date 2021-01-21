#include "Logger.h"
#include "AsyncSerialServer.h"

int main(int argc, char* argv[])
{
    std::streambuf * const coutbuf = std::cout.rdbuf();
    std::clog.rdbuf(new Log("serial-echo", LOG_LOCAL0));
    
    teebuf tee(coutbuf, std::clog.rdbuf());
    std::cout.rdbuf(&tee);

    try
    {
        SerialPortInformation portInformation(argc, argv);

        std::cout << "Opening port: " << portInformation.portName << std::endl;

        boost::asio::io_context io_context;
        AsyncSerialServer serialPort(io_context, portInformation);
        io_context.run();

        std::cout << "Closing port" << std::endl;

    }
    catch(const boost::system::system_error& e)
    {
        std::cerr << "[ERROR]: " << e.what() << ": " << e.code() << " - " << e.code().message() << std::endl;
    }

    std::cout.rdbuf(coutbuf);
    return 0;
}
