#include "utility.h"

void makeVector(std::vector<char> &inputVector, const char *data, std::size_t size)
{
    std::vector<char> createVector;
    inputVector.assign(data, data + size);
}

std::string modemStatusToString(unsigned int status)
{
    if ((status & TIOCM_RTS) && (status & TIOCM_DTR))
        return "RTS+DTR";    
    else if (status & TIOCM_RTS)
        return "RTS";
    else if (status & TIOCM_DTR)
        return "DTR";
    else
        return "NULL";
}
