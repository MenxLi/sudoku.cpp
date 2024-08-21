#include "config.h"
#include "indexer.h"
#include <iostream>
#include <memory>

int main()
{
    auto indexer_p = std::make_unique<Indexer>();
    std::cout << "Hello, World!" << std::endl;
    // Indexer<GRID_SIZE>& indexer = *indexer_p;
    // for (unsigned int i = 0; i < indexer.N; i++)
    // {
    //     for (unsigned int j = 0; j < indexer.N; j++)
    //     {
    //         std::cout << "grid_lookup[" << i << "][" << j << "][0]: " << indexer.grid_lookup[i][j][0] << std::endl;
    //         std::cout << "grid_lookup[" << i << "][" << j << "][1]: " << indexer.grid_lookup[i][j][1] << std::endl;
    //     }
    // }
    return 0;
}