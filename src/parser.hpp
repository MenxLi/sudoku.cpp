#pragma once
#include <cstdlib>
#include <string>

namespace parser{
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

    class CommandlineParser{
    public:
        CommandlineParser(int argc, char* argv[]) : m_argc(argc), m_argv(argv) {}
        std::string get_arg(const char* flag)
        {
            for (int i = 1; i < m_argc; i++)
            {
                if (std::string(m_argv[i]) == flag)
                {
                    if (i + 1 < m_argc)
                    {
                        return m_argv[i + 1];
                    }
                    else
                    {
                        throw std::runtime_error(std::string(flag) + " requires an argument");
                    }
                }
            }
            return "";
        }

    private:
        int m_argc;
        char** m_argv;
    };

}