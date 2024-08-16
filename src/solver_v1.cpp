#include "board.h"
#include "config.h"
#include "solver_v1.h"
#include "util.h"
#include <stdexcept>
#include <stdlib.h>
#include <vector>
#include <random>
#include <algorithm>

#define CANDIDATE_SIZE BOARD_SIZE
#define MAX_FORK_TRAIL MAX_ITER

static bool USE_GUESS;
static bool DETERMINISTIC_GUESS;
static bool HEURISTIC_GUESS;

// #define DEBUG_PRINT(x) std::cout << x << std::endl;
#define DEBUG_PRINT(x);

SolverV1::SolverV1(const Board& board) : Solver(board) {
    // parse environment variables
    USE_GUESS = util::parse_env_i<bool>("SOLVER_USE_GUESS", true);
    DETERMINISTIC_GUESS = util::parse_env_i("SOLVER_DETERMINISTIC_GUESS", false);
    HEURISTIC_GUESS = util::parse_env_i("SOLVER_HEURISTIC_GUESS", true);
    // std::cout << "Config: USE_GUESS=" << USE_GUESS << ", DETERMINISTIC_GUESS=" << DETERMINISTIC_GUESS << ", HEURISTIC_GUESS=" << HEURISTIC_GUESS << std::endl;

    init_cross_map();
    init_candidates_and_count();
};

SolverV1::SolverV1(SolverV1& other) : Solver(other.board()) {
    m_candidates = other.m_candidates;
    // this somehow cause python binding to fail...
    // std::copy(&cross_map[0][0][0], &cross_map[0][0][0] + CANDIDATE_SIZE * BOARD_SIZE * BOARD_SIZE, &m_cross_map[0][0][0]);
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            for (unsigned int k = 0; k < BOARD_SIZE; k++)
            {
                m_cross_map[i][j][k] = other.m_cross_map[i][j][k];
            }
        }
    }
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        m_filled_count[i] = other.m_filled_count[i];
    }
};

void SolverV1::init_cross_map(){
    // initialize the cross map
    // row by row, column by column, and fill in the cross map
    for (unsigned int v = 0; v < CANDIDATE_SIZE; v++)
    {
        val_t value = static_cast<val_t>(v + 1);
        unsigned int m_cross_row[BOARD_SIZE] = {0};
        unsigned int m_cross_col[BOARD_SIZE] = {0};
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

        for (unsigned int i = 0; i < BOARD_SIZE; i++)
        {
            for (unsigned int j = 0; j < BOARD_SIZE; j++)
            {
                m_cross_map[v][i][j] = m_cross_row[i] == 1 || m_cross_col[j] == 1;
            }
        }
    }
};

void SolverV1::init_candidates_and_count(){
    // TODO: maybe use fill_propagate() to initialize all

    auto update_candidate_for = [&](int row, int col){
        if (this->board().get_(row, col) != 0){
            // already has a value
            return;
        }

        for (unsigned int i = 0; i < indexer.N_NEIGHBORS; i++){
            auto offset = indexer.neighbor_index[row][col][i];
            val_t other_val = this->board().get(offset);
            if (other_val != 0){
                m_candidates.get_(row, col, other_val) = 0;
            }
        }
    };


    m_candidates.reset();
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            // init candidate map
            update_candidate_for(i, j);

            // init filled count
            val_t filled_val = this->board().get_(i, j);
            if (filled_val != 0){
                m_filled_count[filled_val - 1] += 1;
            }
        }
    }
};

bool SolverV1::step_by_candidate(){
    bool updated = false;
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            if (update_value_for(i, j)){
                updated = true;
            }
        }
    }
    return updated;
};

bool SolverV1::step_by_crossover(){
    bool ret = false;
    for (val_t i = 1; i <= CANDIDATE_SIZE; i++)
    {
        if (update_by_cross(i)){
            ret = true;
        }
    }
    // for (unsigned int i=0; i<BOARD_SIZE; i++){
    //     for (unsigned int j=0; j<BOARD_SIZE; j++){
    //         if (update_by_cross(i, j)){
    //             ret = true;
    //         }
    //     }
    // }
    return ret;
};

bool SolverV1::step(){
    DEBUG_PRINT("SolverV1::step()");

    if (step_by_candidate()) return true;
    DEBUG_PRINT("SolverV1::step() - step_by_candidate() failed");

    // step by cross is effectively the super-set of step by step_by_candidate
    // but it is less efficient if there are only one candidate left in a cell
    if (step_by_crossover()) return true;
    DEBUG_PRINT("SolverV1::step() - step_by_crossover() failed");

    if (USE_GUESS){
        if (step_by_guess()) return true;
        DEBUG_PRINT("SolverV1::step() - step_by_guess() failed");
    }
    return false;
};

void SolverV1::fill_propagate(unsigned int row, unsigned int col, val_t value){
    board().get_(row, col) = value;

    // clear the candidates for the neighbor cells
    for (unsigned int i = 0; i < indexer.N_NEIGHBORS; i++){
        auto offset = indexer.neighbor_index[row][col][i];
        m_candidates.get(offset)[value - 1] = 0;
    }

    // fill the cross map of the value
    unsigned int c_index = value - 1;
    ASSERT(m_cross_map[c_index][row][col] == 0, "Cross map violation");
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        m_cross_map[value - 1][row][i] = 1;
        m_cross_map[value - 1][i][col] = 1;
    }

    // update the filled count
    unsigned int v_idx = static_cast<unsigned int>(value) - 1;
    m_filled_count[v_idx] += 1;
    ASSERT(m_filled_count[v_idx] <= BOARD_SIZE, "Filled count violation");
};


void SolverV1::refine_candidates(){
    // handles implicit value determination
    // i.e. if a sub-row/col in a grid has multiple candidates for a value,
    // but can uniquely determine the value based on the row/col 
    // (e.g. 57, 75, 375 appears in one row/col of a grid, determins 7 and 5 must be in the same row/col)
    // then we can remove the other candidates from the same total-row/col
    // to be implemented...
};

bool SolverV1::update_value_for(int row, int col){

    if (this->board().get_(row, col) != 0){ return false; }

    val_t candidate_val = 0;
    auto found = m_candidates.remain_x(row, col, 1, &candidate_val);
    if (found == OpState::SUCCESS){
        fill_propagate(row, col, candidate_val);
        return true;
    }
    return false;
};


// this is less efficient than the other implementation...
// because it has to iterate over the grid multiple times!
bool SolverV1::update_by_cross(int row, int col){
    // first, check if the cell is already solved
    if (this->board().get_(row, col) != 0){ return false; }

    bool ret = false;

    // iterate over the candidates,
    // if there is only one candidate that is not marked in the cross map, fill it in
    for (unsigned int v_idx = 0; v_idx < CANDIDATE_SIZE; v_idx++)
    {
        val_t value = static_cast<val_t>(v_idx + 1);

        // if (!m_candidates.get_(row, col, value)) continue;  // this make sure the value is not present in the grid, row or column
        // ASSERT(m_cross_map[v_idx][row][col] == 0, "Cross map violation");   // then it should not be in the same row or column!
        // this equals to the following:
        if (m_cross_map[v_idx][row][col] != 0) continue; // the value is already in the row or column

        // check if the value is also possible in the other cells of the grid
        bool skip_flag = false;
        for (auto offset : indexer.grid_index[row][col])
        {
            if (offset == row * BOARD_SIZE + col) continue;     // skip the aim cell

            unsigned int board_row = offset / BOARD_SIZE;
            unsigned int board_col = offset % BOARD_SIZE;
            if (this->board().get(offset) == value){
                skip_flag = true;
                break;
            } // the value is already in the grid
            if (m_cross_map[v_idx][board_row][board_col] == 0){
                skip_flag = true;
                break;
            } // the value is also possible in the other cells of the grid
        }

        // fill in the value
        if (!skip_flag){
            fill_propagate(row, col, value);
            ret = true;
            break;      // the aimed cell can only have one value
        }
    }
    return ret;
};


bool SolverV1::update_by_cross(val_t value){
    bool ret = false;
    unsigned int value_index = value - 1;

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
                        m_cross_map[value_index][row_base + i][col_base + j] == 0 &&
                        this->board().get_(row_base + i, col_base + j) == 0
                        ){
                        aim_grid_row_idx = i;
                        aim_grid_col_idx = j;
                        count++;
                    }
                }
            }
            if (count == 1){
                fill_propagate(row_base + aim_grid_row_idx, col_base + aim_grid_col_idx, value);
                ret = true;
            }
        }
    }
    return ret;
};

SolverV1 SolverV1::fork(){
    return SolverV1(*this);
};

bool SolverV1::step_by_guess(){
    auto numNeighborUnsolved = [&](unsigned int row, unsigned int col)->unsigned int{
        unsigned int min_count;
        unsigned int row_count = 0;
        unsigned int col_count = 0;
        unsigned int grid_count = 0;

        auto row_item_offsets = indexer.row_index[row];
        auto col_item_offsets = indexer.col_index[col];
        auto grid_item_offsets = indexer.grid_index[row][col];

        for (unsigned int i = 0; i < BOARD_SIZE; i++)
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
        for (unsigned int i = 0; i < BOARD_SIZE; i++)
        {
            for (unsigned int j = 0; j < BOARD_SIZE; j++)
            {
                if (this->board().get_(i, j) != 0){ continue; };     // skip the solved cells

                unsigned int candidate_count = m_candidates.count(i, j);
                if (candidate_count < min_candidate_count){
                    min_candidate_count = candidate_count;
                    min_neighbor_count = numNeighborUnsolved(i, j);
                    best_choice = {static_cast<int>(i), static_cast<int>(j)};
                }
                else if (candidate_count == min_candidate_count){
                    unsigned int neighbor_count = numNeighborUnsolved(i, j);
                    if (neighbor_count < min_neighbor_count){
                        min_neighbor_count = neighbor_count;
                        best_choice = {static_cast<int>(i), static_cast<int>(j)};
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
            for (unsigned int i = 0; i < BOARD_SIZE; i++)
            {
                for (unsigned int j = 0; j < BOARD_SIZE; j++)
                {
                    if (this->board().get_(i, j) == 0){
                        unsolved_cells.push_back({static_cast<int>(i), static_cast<int>(j)});
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
            for (unsigned int i = 0; i < BOARD_SIZE; i++)
            {
                for (unsigned int j = 0; j < BOARD_SIZE; j++)
                {
                    if (this->board().get_(i, j) == 0){
                        best_choice = {static_cast<int>(i), static_cast<int>(j)};
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

    if (HEURISTIC_GUESS){
        // sort the candidate indices by the number of occurences in the board, 
        // choose the one with the least occurences
        // this should facilitateos the backtracking process by increasing the value diversity, 
        // and reduce the chance of getting stuck in a local minimum (which requires more guesses)
        std::sort(candidate_values.begin(), candidate_values.end(), [&](val_t a, val_t b){
            return m_filled_count[a - 1] < m_filled_count[b - 1];
        });
    }
    else if (!DETERMINISTIC_GUESS){
        // shuffle the candidate indices
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(candidate_values.begin(), candidate_values.end(), g);
    }

    // make guesses with backtracking
    for (val_t guess : candidate_values){

        auto forked_solver = this->fork();

        // inherit the iteration counter
        forked_solver.iteration_counter().current = this->iteration_counter().current;

        bool solved;
        try{
            forked_solver.fill_propagate(best_choice.row, best_choice.col, guess);
            solved = forked_solver.solve();
            this->iteration_counter().current = forked_solver.iteration_counter().current;
        }
        catch(std::runtime_error& e){
            this->iteration_counter().current = forked_solver.iteration_counter().current;
            continue;
        }

        if (!solved){ continue; }

        this->board().load_data(forked_solver.board());
        return true;
    }

    // ideally, we should never reach here...
    // unless the board is invalid, trail limit is reached, or the guess is wrong, 
    // when the guess is wrong, the forked solver will throw an exception and we will catch it
    return false;
};