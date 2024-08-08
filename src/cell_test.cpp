#include "cell.h"
#include <iostream>
#include "board.h"
#include "config.h"

std::ostream& operator<<(std::ostream& os, const val_t val)
{
    os << static_cast<int>(val);
    return os;
}
std::ostream& operator<<(std::ostream& os, const Coord& coord)
{
    os << "(" << coord.row << ", " << coord.col << ")";
    return os;
}

void print_cell(Cell& cell)
{
    std::cout << "Cell: " << cell.coord() << std::endl;
    std::cout << "Value: " << cell.value() << std::endl;
    std::cout << "Coordinates: " << cell.coord() << std::endl;
    std::cout << "Grid Coord: " << cell.grid_coord() << std::endl;
    std::cout << "Row: ";
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        std::cout << *cell.row()[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Col: ";
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        std::cout << *cell.col()[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "Grid: ";
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        std::cout << *cell.grid()[i] << " ";
    }
    std::cout << std::endl;
}

int main()
{
    Board board;
    board.load_from_file("./puzzles/1.txt");
    std::cout << board << std::endl;

    CellView cell_view(board);

    Cell cell1(cell_view, {0, 0});
    print_cell(cell1);
    std::cout << "--------" << std::endl;

    Cell cell2(cell_view, {3, 4});
    print_cell(cell2);

    return 0;
};
