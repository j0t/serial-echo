#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind/bind.hpp>

// strand template is used for threading syncronization, it is useful in servers

class printer
{
private:
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
    boost::asio::steady_timer timer1;
    boost::asio::steady_timer timer2;
    int count;

public:
    printer(boost::asio::io_context& io)
        : strand(boost::asio::make_strand(io)),
        timer1(io, boost::asio::chrono::seconds(1)),
        timer2(io, boost::asio::chrono::seconds(1)),
        count(0)
    {
        // Bind the strand template class to the task execution
        this->timer1.async_wait(boost::asio::bind_executor(this->strand,
            boost::bind(&printer::print1, this)));
        
        this->timer2.async_wait(boost::asio::bind_executor(this->strand,
            boost::bind(&printer::print2, this)));
    }

    ~printer()
    {
        std::cout << "Final count is " << this->count << std::endl;
    }

public:
    void print1()
    {
        if(this->count < 10)
        {
            std::cout << "Timer 1: " << this->count << std::endl;
            ++this->count;

            // Loop the task until completion
            this->timer1.expires_at(this->timer1.expiry() + boost::asio::chrono::seconds(1));
            this->timer1.async_wait(boost::asio::bind_executor(this->strand,
                boost::bind(&printer::print1, this)));
        }
    }

    void print2()
    {
        if(this->count < 10)
        {
            std::cout << "Timer 2: " << this->count << std::endl;
            ++this->count;

            this->timer2.expires_at(this->timer2.expiry() + boost::asio::chrono::seconds(1));
            this->timer2.async_wait(boost::asio::bind_executor(this->strand,
                boost::bind(&printer::print2, this)));
        }
    }
};

int main()
{
    boost::asio::io_context io;
    printer p(io);
    boost::thread t(boost::bind(&boost::asio::io_context::run, &io));
    io.run();
    // To enable multithreading
    t.join();

    return 0;
}
