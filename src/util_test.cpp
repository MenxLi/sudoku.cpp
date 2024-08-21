#include "util.h"
#include <iostream>

#define ASSERT_EQ(a, b) if (a != b) { std::cout << "FAIL" << std::endl; } else { std::cout << "PASS" << std::endl; }

using namespace util;

void test_string_split(std::string str, std::string delimiter, std::vector<std::string> expected){
    std::vector<std::string> result = split_string(str, delimiter);
    ASSERT_EQ(result, expected);
}

int main(){
    test_string_split("a,b,c", ",", {"a", "b", "c"} );
    test_string_split("hello, world", ",", {"hello", " world"});
    test_string_split("hello, world,", ",", {"hello", " world", ""});
    test_string_split(",-hello,- world,", ",-", {"", "hello", " world,"});

    if (Factorial<0>::value != 1) { std::cout << "FAIL" << std::endl; } else { std::cout << "PASS" << std::endl; }
    if (factorial<3> != 6) { std::cout << "FAIL" << std::endl; } else { std::cout << "PASS" << std::endl; }

    std::cout << util::n_combinations<17, 2> << std::endl;

    auto ret = combinations<int, 5, 3>({1, 2, 3, 4, 5});
    for (auto& v : ret){
        for (auto& i : v){
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
};
