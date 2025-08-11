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

std::string replace_string(const std::string& str, const std::string& old_s, const std::string& new_s)
{
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(old_s, pos)) != std::string::npos) {
        result.replace(pos, old_s.length(), new_s);
        pos += new_s.length();
    }
    return result;
}

}
