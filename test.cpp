#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

BOOST_AUTO_TEST_SUITE(test_suit1)

BOOST_AUTO_TEST_CASE(test_less_than_buffer_size)
{
    using namespace boost::program_options;

    const char* PORT = "port";

    try
    {
        std::string portName;

        options_description description("Options");
        description.add_options()
            (PORT, value<std::string>(&portName)->default_value("/dev/ttyS0"), "set serial port")
        ;

        variables_map variableMap;
        store(parse_command_line(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv, description), variableMap);
        notify(variableMap);

        if (variableMap.count(PORT))
        {
            std::cout << "Serial port device was set to " << variableMap[PORT].as<std::string>() << "\n";
        }
        else
        {
            std::cout << "Serial port device was set to default\n";
        }

        boost::asio::io_context io_context;
        boost::asio::serial_port serialPort(io_context, portName);

        serialPort.set_option(boost::asio::serial_port_base::baud_rate(9600));
        serialPort.set_option(boost::asio::serial_port_base::character_size(8));
        serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

        boost::array<char, 12> dataBuffer;
        boost::system::error_code error;

        serialPort.write_some(boost::asio::buffer("test_conn"), error);
        serialPort.read_some(boost::asio::buffer(dataBuffer), error);

        BOOST_CHECK_EQUAL(dataBuffer.data(), "test_conn");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(const boost::system::error_code& e)
    {
        std::cerr << "[ERROR]: " << e << '\n';
    }
}

BOOST_AUTO_TEST_CASE(test_more_than_buffer_size)
{
    using namespace boost::program_options;

    const char* PORT = "port";

    try
    {
        std::string portName;

        options_description description("Options");
        description.add_options()
            (PORT, value<std::string>(&portName)->default_value("/dev/ttyS0"), "set serial port")
        ;

        variables_map variableMap;
        store(parse_command_line(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv, description), variableMap);
        notify(variableMap);

        if (variableMap.count(PORT))
        {
            std::cout << "Serial port device was set to " << variableMap[PORT].as<std::string>() << "\n";
        }
        else
        {
            std::cout << "Serial port device was set to default\n";
        }

        boost::asio::io_context io_context;
        boost::asio::serial_port serialPort(io_context, portName);

        serialPort.set_option(boost::asio::serial_port_base::baud_rate(9600));
        serialPort.set_option(boost::asio::serial_port_base::character_size(8));
        serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

        boost::array<char, 12> dataBuffer;
        boost::system::error_code error;

        serialPort.write_some(boost::asio::buffer("test_connection"), error);
        serialPort.read_some(boost::asio::buffer(dataBuffer), error);

        BOOST_CHECK_EQUAL(dataBuffer.data(), "test_connection");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(const boost::system::error_code& e)
    {
        std::cerr << "[ERROR]: " << e << '\n';
    }
}

BOOST_AUTO_TEST_CASE(test_non_ASCII_or_null)
{
    using namespace boost::program_options;

    const char* PORT = "port";

    try
    {
        std::string portName;

        options_description description("Options");
        description.add_options()
            (PORT, value<std::string>(&portName)->default_value("/dev/ttyS0"), "set serial port")
        ;

        variables_map variableMap;
        store(parse_command_line(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv, description), variableMap);
        notify(variableMap);

        if (variableMap.count(PORT))
        {
            std::cout << "Serial port device was set to " << variableMap[PORT].as<std::string>() << "\n";
        }
        else
        {
            std::cout << "Serial port device was set to default\n";
        }

        boost::asio::io_context io_context;
        boost::asio::serial_port serialPort(io_context, portName);

        serialPort.set_option(boost::asio::serial_port_base::baud_rate(9600));
        serialPort.set_option(boost::asio::serial_port_base::character_size(8));
        serialPort.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        serialPort.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serialPort.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

        boost::array<char, 12> dataBuffer;
        boost::system::error_code error;

        serialPort.write_some(boost::asio::buffer("tēst_\0čonn"), error);
        serialPort.read_some(boost::asio::buffer(dataBuffer), error);

        BOOST_CHECK_EQUAL(dataBuffer.data(), "tēst_\0čonn");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(const boost::system::error_code& e)
    {
        std::cerr << "[ERROR]: " << e << '\n';
    }
}

BOOST_AUTO_TEST_SUITE_END()
