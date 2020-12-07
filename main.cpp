#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

class printer
{
private:
    boost::asio::steady_timer timer;
    int count;

public:
    printer(boost::asio::io_context& io)
        : timer(io, boost::asio::chrono::seconds(1)), count(0)
    {
        // Do the task async, which is print
        this->timer.async_wait(boost::bind(&printer::print, this));
    }

    ~printer()
    {
        std::cout << "Final count is " << this->count << std::endl;
    }

public:
    void print()
    {
        if (this->count < 5)
        {
            std::cout << this->count << std::endl;
            ++this->count;

            // Add a some waiting time so that the counter doesnt fall out of sync
            this->timer.expires_at(this->timer.expiry() + boost::asio::chrono::seconds(1));
            // Check if the task is done executing, which is print or do it
            this->timer.async_wait(boost::bind(&printer::print, this));
        }
    }
};

int main()
{
    // The context is the link to the operating systems I/O services.
    boost::asio::io_context io;
    printer p(io);
    // io needs to have some work to do for it to work in a async manner
    io.run();

    return 0;
}
