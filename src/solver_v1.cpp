#include "board.h"
#include "config.h"
#include "solver_v1.h"
#include <memory>
#include <stdexcept>
#include <stdlib.h>

#define CANDIDATE_SIZE BOARD_SIZE
#define MAX_FORK_TRAIL 1e4

bool ENV_USE_GUESS;

// #define DEBUG_PRINT(x) std::cout << x << std::endl;
#define DEBUG_PRINT(x);

SolverV1::SolverV1(Board& board) : Solver(board) {
    clear_candidates();

    // parse environment variable
    char* ENV_USE_GUESS_STR = getenv("GUESS");
    if (ENV_USE_GUESS_STR == nullptr){
        ENV_USE_GUESS = false;
    }
    else{
        ENV_USE_GUESS = std::stoi(ENV_USE_GUESS_STR);
    }
};

bool SolverV1::step_by_candidate(){
    update_candidates();
    bool ret = update_values();
    clear_candidates();
    return ret;
};

bool SolverV1::step_by_crossover(){
    bool ret = false;
    for (val_t i = 1; i <= BOARD_SIZE; i++)
    {
        if (update_by_cross(i)){
            ret = true;
        }
    }
    return ret;
};

bool SolverV1::step_by_guess(){
    update_candidates();
    bool ret = update_by_guess();
    clear_candidates();
    return ret;
};

bool SolverV1::step(){
    DEBUG_PRINT("SolverV1::step()");
    if (step_by_candidate()) return true;
    DEBUG_PRINT("SolverV1::step() - step_by_candidate() failed");
    if (step_by_crossover()) return true;
    DEBUG_PRINT("SolverV1::step() - step_by_crossover() failed");

    if (ENV_USE_GUESS){
        if (step_by_guess()) return true;
        DEBUG_PRINT("SolverV1::step() - step_by_guess() failed");
    }
    return false;
};

void SolverV1::update_candidate_for(int row, int col){
    Cell& cell = this->cell(row, col);
    if (cell.value() != 0){
        // already has a value
        return;
    }

    // remove candidates from the same row, column, and grid
    for (int idx = 0; idx < cell.neighbor_count; idx++)
    {
        val_t other_val = *cell.neighbor()[idx];
        if (other_val != 0){
            m_candidates[row][col][other_val - 1] = 0;
        }
    }
};

void SolverV1::update_candidates(){
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            update_candidate_for(i, j);
        }
    }
};

void SolverV1::clear_candidates(){
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            for (int k = 0; k < CANDIDATE_SIZE; k++)
            {
                m_candidates[i][j][k] = k + 1;
            }
        }
    }
};

bool SolverV1::update_value_for(int row, int col){
    Cell& cell = this->cell(row, col);
    if (cell.value() != 0){
        return false;
    }

    int candidate_count = 0;
    val_t candidate_val = 0;
    for (int i = 0; i < CANDIDATE_SIZE; i++)
    {
        if (m_candidates[row][col][i] != 0){
            candidate_count++;
            candidate_val = m_candidates[row][col][i];
        }
    }

    if (candidate_count == 0){
        throw std::runtime_error("No candidate left for cell, bad board?");
    }

    if (candidate_count == 1){
        cell.value() = candidate_val;
        return true;
    }
    return false;
};

bool SolverV1::update_values(){
    bool updated = false;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (update_value_for(i, j)){
                updated = true;
            }
        }
    }
    return updated;
};

bool SolverV1::update_by_cross(val_t value){
    bool ret = false;
    // first, clear the cross map
    clear_cross_map();

    // iterate over the board marking cells that have the value
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (this->cell(i, j).value() == value){
                m_cross_map[i][j] = 1;
            }
        }
    }

    // iterate over the map, 
    // row by row, column by column, and fill in the value where there is a 1
    for (unsigned int row_idx = 0; row_idx < BOARD_SIZE; row_idx ++ ){
        for (unsigned int col_idx = 0; col_idx < BOARD_SIZE; col_idx++){
            if (m_cross_map[row_idx][col_idx] == 1){
                m_cross_row[row_idx] += 1;
                m_cross_col[col_idx] += 1;
            }
        }
    }

    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        if (m_cross_row[i] > 1 || m_cross_col[i] > 1){
            throw std::runtime_error("Axis count violation");
        }
    }

    // update the cross map with the row and column maps
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            m_cross_map[i][j] = m_cross_row[i] == 1 || m_cross_col[j] == 1;
        }
    }

    // iterate over grid, 
    // if there is only one cell in the grid that is unsoved and not marked, fill it in
    for (unsigned int g_row = 0; g_row < GRID_SIZE; g_row++)
    {
        for (unsigned int g_col = 0; g_col < GRID_SIZE; g_col++)
        {
            unsigned int count = 0;
            unsigned int row_base = g_row * GRID_SIZE;
            unsigned int col_base = g_col * GRID_SIZE;

            unsigned int aim_grid_row_idx = 0;
            unsigned int aim_grid_col_idx = 0;

            // iterate over the grid, 
            bool skip_grid_flag = false;
            for (unsigned int i = 0; i < GRID_SIZE; i++)
            {
                for (unsigned int j = 0; j < GRID_SIZE; j++)
                {
                    if (this->cell(row_base + i, col_base + j).value() == value){
                        // the value is already in the grid, skip this grid!
                        skip_grid_flag = true;
                        count = 0; // reset count, make sure we don't fill in the grid
                    }
                    if (
                        !skip_grid_flag &&
                        m_cross_map[row_base + i][col_base + j] == 0 && 
                        this->cell(row_base + i, col_base + j).value() == 0
                        ){
                        aim_grid_row_idx = i;
                        aim_grid_col_idx = j;
                        count++;
                    }
                }
            }
            if (count == 1){
                this->cell(row_base + aim_grid_row_idx, col_base + aim_grid_col_idx).value() = value;
                ret = true;
            }
        }
    }
    return ret;
};

void SolverV1::clear_cross_map(){
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        // tricky, use because row and col are the same size
        m_cross_row[i] = 0;
        m_cross_col[i] = 0;

        for (int j = 0; j < BOARD_SIZE; j++)
        {
            m_cross_map[i][j] = 0;
        }
    }
};

std::tuple<std::unique_ptr<SolverV1>, std::unique_ptr<Board>> SolverV1::fork(){
    std::unique_ptr<Board> new_board(new Board());
    new_board->load_data(this->board());
    std::unique_ptr<SolverV1> ret(new SolverV1(*new_board));
    return std::make_tuple(std::move(ret), std::move(new_board));
};

bool SolverV1::update_by_guess(){
    auto numNeighborUnsolved = [](Cell& cell){
        unsigned int min_count;
        unsigned int row_count = 0;
        unsigned int col_count = 0;
        unsigned int grid_count = 0;

        for (int i = 0; i < BOARD_SIZE; i++)
        {
            if (*cell.row()[i] == 0) row_count++;
            if (*cell.col()[i] == 0) col_count++;
            if (*cell.grid()[i] == 0) grid_count++;
        }
        min_count = row_count;
        if (col_count < min_count) min_count = col_count;
        if (grid_count < min_count) min_count = grid_count;
        return min_count;
    };

    // find the best cell to guess, 
    // by finding:
    // 1. the cell with the least number of candidates
    // 2. the cell with the least number of unsolved neighbors
    Cell* best_choice = nullptr;
    unsigned int min_candidate_count = 1e4;
    unsigned int min_neighbor_count = 1e4;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            Cell& cell = this->cell(i, j);
            if (cell.value() != 0){
                continue;
            };

            unsigned int candidate_count = 0;
            for (int k = 0; k < CANDIDATE_SIZE; k++)
            {
                if (m_candidates[i][j][k] != 0){
                    candidate_count++;
                }
            }
            if (candidate_count < min_candidate_count){
                min_candidate_count = candidate_count;
                min_neighbor_count = numNeighborUnsolved(cell);
                best_choice = &cell;
            }
            else if (candidate_count == min_candidate_count){
                unsigned int neighbor_count = numNeighborUnsolved(cell);
                if (neighbor_count < min_neighbor_count){
                    min_neighbor_count = neighbor_count;
                    best_choice = &cell;
                }
            }
            else{
                // do nothing if the candidate count is larger
            }
        }
    }

    // random choose a candidate in the best choice location
    for(int k = 0; k < CANDIDATE_SIZE; k++){
        if (m_candidates[best_choice->coord().row][best_choice->coord().col][k] != 0){
            // make a guess
            auto [forked_solver, forked_board] = this->fork();
            val_t guess = m_candidates[best_choice->coord().row][best_choice->coord().col][k];
            forked_solver->cell(best_choice->coord().row, best_choice->coord().col).value() = guess;

            unsigned int fork_trail_limit = this->iteration_counter().limit - this->iteration_counter().current;
            if (fork_trail_limit > MAX_FORK_TRAIL){
                fork_trail_limit = MAX_FORK_TRAIL;
            }

            bool solved;
            try{
                solved = forked_solver->solve(fork_trail_limit);
                this->iteration_counter().current += forked_solver->iteration_counter().current;
            }
            catch(std::runtime_error& e){
                this->iteration_counter().current += forked_solver->iteration_counter().current;
                continue;
            }

            if (!solved){ continue; }

            this->board().load_data(forked_solver->board());
            return true;
        }
    }
    return false;
};