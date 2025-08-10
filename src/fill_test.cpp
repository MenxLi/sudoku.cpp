#include "board.h"
#include "fill.h"

int main(){
    auto b = Board();
    gen::fill_board(b, gen::FillStrategy::NAIVE);

    std::cout << "Filled board:" << std::endl;
    std::cout << b << std::endl;

    auto dist = gen::UniformIntDist<1, 9>();
    std::cout << "Random samples from UniformIntDist<1, 9>:" << std::endl;
    for (int i = 0; i < 10; ++i) {
        std::cout << dist.sample() << " ";
    }
    std::cout << std::endl;

    std::cout << "Testing UniformIntDist second instance:" << std::endl;
    auto dist2 = gen::UniformIntDist<1, 9>();
    for (int i = 0; i < 10; ++i) {
        std::cout << dist2.sample() << " ";
    }
    std::cout << std::endl;
}