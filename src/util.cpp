#include "util.h"
#include "config.h"

namespace util{


std::vector<std::string> split_string(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> result;
    unsigned int start = 0;
    while(true)
    {
        auto end = str.find(delimiter, start);
        if (end == std::string::npos)
        {
            result.push_back(str.substr(start));
            break;
        }
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.size();
    }
    return result;
}

}
