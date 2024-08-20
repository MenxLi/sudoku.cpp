#pragma once
#include <cstdlib>
#include <string>

namespace util{
    template <typename T>
    T parse_env_i(const char* env_name, T default_value)
    {
        const char* env_value = std::getenv(env_name);
        if (env_value == nullptr) { return default_value; }
        return static_cast<T>(std::stoi(env_value));
    }

    template <typename T>
    T parse_env_f(const char* env_name, T default_value)
    {
        const char* env_value = std::getenv(env_name);
        if (env_value == nullptr) { return default_value; }
        return static_cast<T>(std::stof(env_value));
    }


}