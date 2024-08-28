#pragma once
#include <cstdlib>
#include <string>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <tuple>

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
        inline CommandlineParser(int argc, char* argv[]) : m_argc(argc), m_argv(argv) {}

        inline std::string prog_name()
        {
            return m_argv[0];
        }

        inline void set_help_message(std::string help_message){
            m_help_message = help_message;
        }

        inline bool has_subparser(const char* name){
            if (m_argc < 2) return false;
            if (std::string(m_argv[1]) == name){ return true; }
            else{ return false; }
        }

        inline bool has_arg(const char* flag)
        {
            for (int i = 1; i < m_argc; i++)
            {
                if (std::string(m_argv[i]) == flag)
                {
                    return true;
                }
            }
            return false;
        }

        inline std::string get_arg(const char* flag)
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

        template <typename T>
        T parse_arg(const char* flag, T default_value)
        {
            std::string arg = get_arg(flag);
            if (arg.empty()) { return default_value; }

            if constexpr (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
            {
                return static_cast<T>(std::stoi(arg));
            }
            else if constexpr (std::is_same<T, float>::value || std::is_same<T, double>::value)
            {
                return static_cast<T>(std::stof(arg));
            }
            else if constexpr (std::is_same<T, std::string>::value)
            {
                return arg;
            }
            else
            {
                throw std::runtime_error("Unsupported type");
            }
        }
        template <typename T>
        T parse_arg(const char* flag)
        {
            if (!has_arg(flag)) { throw std::runtime_error(std::string(flag) + " is required"); }
            return parse_arg<T>(flag, T());
        }

        inline bool parse_flag(const char* flag)
        {
            return has_arg(flag);
        }

        inline void check_help_exit(){
            if (parse_flag("-h") || parse_flag("--help"))
            {
                std::cout << m_help_message << std::endl;
                exit(0);
            }
        }

        inline void check_help_exit(std::string help_message)
        {
            if (parse_flag("-h") || parse_flag("--help"))
            {
                std::cout << help_message << std::endl;
                exit(0);
            }
        }

    private:
        int m_argc;
        char** m_argv;
        std::string m_help_message;
    };

}