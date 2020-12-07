#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

// Print the count of asyn tasks done, after a set amount of time.
void print(const boost::system::error_code&, boost::asio::steady_timer* t, int* count)
{
    if (*count < 5)
    {
        std::cout << *count << std::endl;
        ++(*count); 

        // Wait to expire before ccontinue so that we are in sync
        t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));
        // Do the asyn task
        t->async_wait(boost::bind(print, boost::asio::placeholders::error, t, count));
    }
}

int main()
{
    // Create context
    boost::asio::io_context io;

    int count = 0;
    // Create a timer
    boost::asio::steady_timer t(io, boost::asio::chrono::seconds(1));
    
    // Pass the funciton to do the task of printing at intervals 6 times
    t.async_wait(boost::bind(print, boost::asio::placeholders::error, &t, &count));
    
    // Need to add this so taht the system know that we are waiting for aysn tasks to be done.
    io.run();

    std::cout << "Final count is " << count << std::endl;

    return 0;
}
