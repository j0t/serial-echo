#include "utility.h"

#define countof(x) sizeof(x) / sizeof(x[0])
#define str(X) #X
#define ITEM(X) std::make_pair(TIOCM_##X,str(X))

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
    typedef std::pair<unsigned int, const char *> pair_t;
    const pair_t pairs[] = {
          ITEM(DTR)
        , ITEM(RTS)
        , ITEM(CTS)
        , ITEM(CAR)
        , ITEM(DSR)
    };
    std::string result = "(";
	for(size_t i = 0; i < countof(pairs); ++i) {
        if( data & pairs[i].first ) {
            if( result.size() > 1 )
                result += '|';
   		    result += pairs[i].second;
        }
	}
    result += ')';
	return result;
}
