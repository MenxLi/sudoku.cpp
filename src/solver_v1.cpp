#include "board.h"
#include "config.h"
#include "solver_v1.h"
#include "util.h"
#include <memory>
#include <stdexcept>
#include <stdlib.h>
#include <vector>
#include <random>
#include <algorithm>

#define CANDIDATE_SIZE BOARD_SIZE
#define MAX_FORK_TRAIL MAX_ITER

bool USE_GUESS;
bool DETERMINISTIC_GUESS;
bool HEURISTIC_GUESS;

// #define DEBUG_PRINT(x) std::cout << x << std::endl;
#define DEBUG_PRINT(x);

SolverV1::SolverV1(Board& board) : Solver(board) {
    // parse environment variables
    USE_GUESS = util::parse_env_i<bool>("SOLVER_NO_GUESS", true);
    DETERMINISTIC_GUESS = util::parse_env_i("SOLVER_DETERMINISTIC_GUESS", false);
    HEURISTIC_GUESS = util::parse_env_i("SOLVER_HEURISTIC_GUESS", true);
    // std::cout << "Config: USE_GUESS=" << USE_GUESS << ", DETERMINISTIC_GUESS=" << DETERMINISTIC_GUESS << ", HEURISTIC_GUESS=" << HEURISTIC_GUESS << std::endl;
};

bool SolverV1::step_by_candidate(){
    update_candidates();
    bool ret = update_values();
    return ret;
};

bool SolverV1::step_by_crossover(){
    bool ret = false;
    for (val_t i = 1; i <= CANDIDATE_SIZE; i++)
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
    return ret;
};

bool SolverV1::step(){
    DEBUG_PRINT("SolverV1::step()");
    if (step_by_candidate()) return true;
    DEBUG_PRINT("SolverV1::step() - step_by_candidate() failed");
    if (step_by_crossover()) return true;
    DEBUG_PRINT("SolverV1::step() - step_by_crossover() failed");

    if (USE_GUESS){
        if (step_by_guess()) return true;
        DEBUG_PRINT("SolverV1::step() - step_by_guess() failed");
    }
    return false;
};

void SolverV1::update_candidate_for(int row, int col){
    if (this->board().get_(row, col) != 0){
        // already has a value
        return;
    }

    for (int i = 0; i < indexer.N_NEIGHBORS; i++){
        auto offset = indexer.neighbor_index[row][col][i];
        val_t other_val = this->board().get(offset);
        if (other_val != 0){
            m_candidates.get_(row, col, other_val) = 0;
        }
    }

};

void SolverV1::update_candidates(){
    reset_candidates();
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            update_candidate_for(i, j);
        }
    }
};

void SolverV1::refine_candidates(){
    // handles implicit value determination
    // i.e. if a sub-row/col in a grid has multiple candidates for a value,
    // but can uniquely determine the value based on the row/col 
    // (e.g. 57, 75, 375 appears in one row/col of a grid, determins 7 and 5 must be in the same row/col)
    // then we can remove the other candidates from the same total-row/col
    // to be implemented...
};

void SolverV1::reset_candidates(){
    m_candidates.reset();
};

bool SolverV1::update_value_for(int row, int col){

    val_t& val = this->board().get_(row, col);
    if (val != 0){ return false; }

    val_t candidate_val = 0;
    bool found = m_candidates.remain_x(row, col, 1, &candidate_val);
    if (found){
        val = candidate_val;
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

    unsigned int m_cross_row[BOARD_SIZE] = {0};
    unsigned int m_cross_col[BOARD_SIZE] = {0};
    unsigned int m_cross_map[BOARD_SIZE][BOARD_SIZE] = {0};

    // iterate over the map, 
    // row by row, column by column, and fill in the cross map
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            if (this->board().get_(i, j) == value){
                m_cross_row[i] += 1;
                m_cross_col[j] += 1;
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
                    if (this->board().get_(row_base + i, col_base + j) == value){
                        // the value is already in the grid, skip this grid!
                        skip_grid_flag = true;
                        count = 0; // reset count, make sure we don't fill in the grid
                    }
                    if (
                        !skip_grid_flag &&
                        m_cross_map[row_base + i][col_base + j] == 0 &&
                        this->board().get_(row_base + i, col_base + j) == 0
                        ){
                        aim_grid_row_idx = i;
                        aim_grid_col_idx = j;
                        count++;
                    }
                }
            }
            if (count == 1){
                this->board().get_(row_base + aim_grid_row_idx, col_base + aim_grid_col_idx) = value;
                ret = true;
            }
        }
    }
    return ret;
};

std::tuple<std::unique_ptr<SolverV1>, std::unique_ptr<Board>> SolverV1::fork(){
    std::unique_ptr<Board> new_board(new Board());
    new_board->load_data(this->board());
    std::unique_ptr<SolverV1> ret(new SolverV1(*new_board));
    return std::make_tuple(std::move(ret), std::move(new_board));
};

bool SolverV1::update_by_guess(){
    auto numNeighborUnsolved = [&](unsigned int row, unsigned int col)->unsigned int{
        unsigned int min_count;
        unsigned int row_count = 0;
        unsigned int col_count = 0;
        unsigned int grid_count = 0;

        auto row_item_offsets = indexer.row_index[row];
        auto col_item_offsets = indexer.col_index[col];
        auto grid_item_offsets = indexer.grid_index[row][col];

        for (int i = 0; i < BOARD_SIZE; i++)
        {
            if (this->board().get(row_item_offsets[i]) == 0) row_count++;
            if (this->board().get(col_item_offsets[i]) == 0) col_count++;
            if (this->board().get(grid_item_offsets[i]) == 0) grid_count++;
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
    auto get_heuristic_choice = [&]()->Coord {
        Coord best_choice;
        unsigned int min_candidate_count = 1e4;
        unsigned int min_neighbor_count = 1e4;
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                val_t& val = board().get_(i, j);
                if (val != 0){ continue; };     // skip the solved cells

                unsigned int candidate_count = m_candidates.count(i, j);
                if (candidate_count < min_candidate_count){
                    min_candidate_count = candidate_count;
                    min_neighbor_count = numNeighborUnsolved(i, j);
                    best_choice = {i, j};
                }
                else if (candidate_count == min_candidate_count){
                    unsigned int neighbor_count = numNeighborUnsolved(i, j);
                    if (neighbor_count < min_neighbor_count){
                        min_neighbor_count = neighbor_count;
                        best_choice = {i, j};
                    }
                }
                else{
                    // do nothing if the candidate count is larger
                }
            }
        }
        return best_choice;
    };

    // choose a cell to guess
    Coord best_choice;
    if (HEURISTIC_GUESS){
        best_choice = get_heuristic_choice();
    }
    else{
        if (!DETERMINISTIC_GUESS){
            // choose a random cell to guess
            std::vector<Coord> unsolved_cells;
            for (int i = 0; i < BOARD_SIZE; i++)
            {
                for (int j = 0; j < BOARD_SIZE; j++)
                {
                    if (this->board().get_(i, j) == 0){
                        unsolved_cells.push_back({i, j});
                    }
                }
            }
            // random guess
            int random_idx = rand() % unsolved_cells.size();
            best_choice = unsolved_cells[random_idx];
        }
        else{
            // choose the first unsolved cell
            bool _found = false;
            for (int i = 0; i < BOARD_SIZE; i++)
            {
                for (int j = 0; j < BOARD_SIZE; j++)
                {
                    if (this->board().get_(i, j) == 0){
                        best_choice = {i, j};
                        _found = true;
                        break;
                    }
                }
                if (_found) break;
            }
        }
    }

    // choose a candidate in the best choice location

    // collect the indices of the candidates where the value is not 0
    std::vector<val_t> candidate_values;
    // candidate_values.reserve(CANDIDATE_SIZE);
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        val_t val = static_cast<val_t>(i + 1);
        if (m_candidates.get_(best_choice.row, best_choice.col, val)){
            candidate_values.push_back(val);
        }
    }

    if (!DETERMINISTIC_GUESS){
        // shuffle the candidate indices
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(candidate_values.begin(), candidate_values.end(), g);
    }

    // make guesses with backtracking
    for (val_t guess : candidate_values){

        auto [forked_solver, forked_board] = this->fork();
        forked_solver->board().get_(best_choice.row, best_choice.col) = guess;

        // inherit the iteration counter
        forked_solver->iteration_counter().current = this->iteration_counter().current;

        bool solved;
        try{
            solved = forked_solver->solve();
            this->iteration_counter().current = forked_solver->iteration_counter().current;
        }
        catch(std::runtime_error& e){
            this->iteration_counter().current = forked_solver->iteration_counter().current;
            continue;
        }

        if (!solved){ continue; }

        this->board().load_data(forked_solver->board());
        return true;
    }

    // ideally, we should never reach here...
    // unless the board is invalid, trail limit is reached, or the guess is wrong, 
    // when the guess is wrong, the forked solver will throw an exception and we will catch it
    throw std::runtime_error("No solution found by guessing");
    return false;
};