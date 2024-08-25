#pragma once

#include <cstring>
#include <vector>
#include <array>
#include <string>
#include <random>

namespace util{
    std::vector<std::string> split_string(const std::string& str, const std::string& delimiter);

    template <typename T, unsigned int size_>
    class SizedArray
    {
    public:
        void push(T value)
        {
            if (m_size >= size_) { return; }
            m_data[m_size] = value;
            m_size++;
        }
        T* data()
        {
            return m_data;
        }
        unsigned int size() const
        {
            return m_size;
        }
        void reverse()
        {
            for (unsigned int i = 0; i < m_size / 2; i++)
            {
                T temp = m_data[i];
                m_data[i] = m_data[m_size - i - 1];
                m_data[m_size - i - 1] = temp;
            }
        }
        bool operator==(const SizedArray<T, size_>& other) const
        {
            return std::memcmp(m_data, other.m_data, size_ * sizeof(T)) == 0;
        }
        T& operator[](unsigned int idx)
        {
            return m_data[idx];
        }
    private:
        T m_data[size_] = { 0 };
        unsigned int m_size = 0;
    };

    // use bubble sort to sort the first size elements in the array, 
    // returns the sorted array in ascending order
    // only use this for very small arrays
    template <typename T>
    void sort_array_bubble(T* arr, unsigned int size, bool (*compare_fn)(T, T) = [](T a, T b) { return a < b; })
    {
        while (true) {
            bool swapped = false;
            for (unsigned int i = 1; i < size; i++) {
                if (compare_fn(arr[i], arr[i - 1])) {
                    T temp = arr[i];
                    arr[i] = arr[i - 1];
                    arr[i - 1] = temp;
                    swapped = true;
                }
            }
            if (!swapped) { break; }
        }
    }

    static std::random_device dev;
    static std::mt19937 rng(dev());
    // suffle the first size elements of an array 
    // using the Fisher-Yates algorithm
    // use this for very small arrays
    template <typename T>
    void shuffle_array(T* arr, unsigned int size)
    {
        std::uniform_int_distribution<std::mt19937::result_type> dist(0, size - 1);
        for (unsigned int i = 0; i < size; i++)
        {
            unsigned int j = dist(rng);
            T temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    template <unsigned int N>
    struct Factorial
    {
        static constexpr unsigned int value = N * Factorial<N - 1>::value;
    };

    template <>
    struct Factorial<0>
    {
        static constexpr unsigned int value = 1;
    };

    template <unsigned int N, unsigned int X>
    struct N_Permutations
    {
        static constexpr unsigned int value = Factorial<N>::value / Factorial<N - X>::value;
    };

    // return the number of combinations of N choose X
    template <unsigned int N, unsigned int X>
    struct N_Combinations
    {
        static constexpr unsigned int value = N_Permutations<N, X>::value / Factorial<X>::value;
    };

    template <unsigned int N>
    constexpr unsigned int factorial = Factorial<N>::value;

    template <unsigned int N, unsigned int X>
    constexpr unsigned int n_permutations = N_Permutations<N, X>::value;

    template <unsigned int N, unsigned int X>
    constexpr unsigned int n_combinations = N_Combinations<N, X>::value;

    template <typename T, unsigned int N, unsigned int X>
    constexpr std::array<std::array<T, X>, n_combinations<N, X>> combinations(const std::array<T, N>& arr)
    {
        static_assert(N >= X, "N must be greater than or equal to X");
        static_assert(X >= 1, "X must be greater than or equal to 1");
        const unsigned int res_n = n_combinations<N, X>;
        std::array<std::array<T, X>, res_n> result = {};
        if constexpr (N == X)     // only one result, just copy the array
        {
            for (unsigned int i = 0; i < N; i++)
            {
                result[0][i] = arr[i];
            }
            return result;
        }

        if constexpr (X==1){      // each element is a result
            for (unsigned int i = 0; i < N; i++)
            {
                result[i][0] = arr[i];
            }
            return result;
        }

        if constexpr (X > 1 && N > X){
            std::array<T, N - 1> sub_arr = {0};
            for (unsigned int i = 0; i < N - 1; i++)
            {
                sub_arr[i] = arr[i + 1];
            }

            // if the first element is in the result, then the rest of the elements are combinations of N-1, X-1
            auto n_sub_res_a = n_combinations<N - 1, X - 1>;
            auto sub_result_a = combinations<T, N - 1, X - 1>(sub_arr);
            for (unsigned int i = 0; i < n_sub_res_a; i++)
            {
                result[i][0] = arr[0];
                for (unsigned int j = 0; j < X - 1; j++)
                {
                    result[i][j + 1] = sub_result_a[i][j];
                }
            }

            // if the first element is not in the result, then the rest of the elements are combinations of N-1, X
            auto n_sub_res_b = n_combinations<N - 1, X>;
            auto sub_result_b = combinations<T, N - 1, X>(sub_arr);
            for (unsigned int i = 0; i < n_sub_res_b; i++)
            {
                for (unsigned int j = 0; j < X; j++)
                {
                    result[i + n_sub_res_a][j] = sub_result_b[i][j];
                }
            }

            return result;
        }
    }

    template <unsigned int N>
    constexpr std::array<unsigned int, N> range()
    {
        std::array<unsigned int, N> result = {0};
        for (unsigned int i = 0; i < N; i++)
        {
            result[i] = i;
        }
        return result;
    }

}
