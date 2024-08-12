#include "config.h"
#include <vector>
#include <string>

namespace util{
    std::vector<std::string> split_string(const std::string& str, const std::string& delimiter);

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

    // use bubble sort to sort the first size elements in the array, 
    // returns the sorted array in ascending order
    // only use this for very small arrays
    template <typename T>
    void sort_array_bubble(T* arr, unsigned int size)
    {
        while (true) {
            bool swapped = false;
            for (unsigned int i = 1; i < size; i++) {
                if (arr[i - 1] > arr[i]) {
                    T temp = arr[i - 1];
                    arr[i - 1] = arr[i];
                    arr[i] = temp;
                    swapped = true;
                }
            }
            if (!swapped) { break; }
        }
    }

    // suffle the first size elements of an array 
    // using the Fisher-Yates algorithm
    // use this for very small arrays
    template <typename T>
    void shuffle_array(T* arr, unsigned int size)
    {
        for (unsigned int i = 0; i < size; i++)
        {
            unsigned int j = rand() % size;
            T temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

}