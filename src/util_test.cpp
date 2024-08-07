#include "util.h"
#include <iostream>

using namespace util;

void test_string_split(std::string str, std::string delimiter, std::vector<std::string> expected){
    std::vector<std::string> result = split_string(str, delimiter);
    if (result == expected){
        std::cout << "PASS" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
    }
}

int main(){
    test_string_split("a,b,c", ",", {"a", "b", "c"} );
    test_string_split("hello, world", ",", {"hello", " world"});
    test_string_split("hello, world,", ",", {"hello", " world", ""});
    test_string_split(",-hello,- world,", ",-", {"", "hello", " world,"});
};
