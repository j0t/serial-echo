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

std::string modemDataTypesToString(int data)
{
    std::string returnString;

    returnString = " (";

    if (data & TIOCM_CTS)
        returnString.append("CTS|");

    if (data & TIOCM_DSR)
        returnString.append("DSR|");

    if (data & TIOCM_CD)
        returnString.append("CD|");

    if (data & TIOCM_RTS)
        returnString.append("RTS|");

    if (data & TIOCM_DTR)
        returnString.append("DTR");

    returnString.append(")");

    return returnString;
}
