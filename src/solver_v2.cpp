#include "board.h"
#include "config.h"
#include "solver.h"
#include "solver_v2.h"
#include "util.h"
#include <memory>
#include <stdexcept>
#include <stdlib.h>

#define CANDIDATE_SIZE BOARD_SIZE
#define MAX_FORK_TRAIL MAX_ITER

static bool USE_GUESS;
static bool DETERMINISTIC_GUESS;
static bool HEURISTIC_GUESS;

// #define DEBUG_PRINT(x) std::cout << x << std::endl;
#define DEBUG_PRINT(x);

SolverV2::SolverV2(const Board& board) : Solver(board) {
    // parse environment variables
    USE_GUESS = util::parse_env_i<bool>("SOLVER_USE_GUESS", true);
    DETERMINISTIC_GUESS = util::parse_env_i("SOLVER_DETERMINISTIC_GUESS", false);
    HEURISTIC_GUESS = util::parse_env_i("SOLVER_HEURISTIC_GUESS", true);
    // std::cout << "Config: USE_GUESS=" << USE_GUESS << ", DETERMINISTIC_GUESS=" << DETERMINISTIC_GUESS << ", HEURISTIC_GUESS=" << HEURISTIC_GUESS << std::endl;

    init_states();
};

SolverV2::SolverV2(SolverV2& other) : Solver(other.board()) {
    m_candidates = other.m_candidates;

    // copy unit fill state
    for (unsigned int i=0; i<BOARD_SIZE; i++){
        for (unsigned int v_idx=0; v_idx<CANDIDATE_SIZE; v_idx++){
            m_row_value_state[i][v_idx] = other.m_row_value_state[i][v_idx];
            m_col_value_state[i][v_idx] = other.m_col_value_state[i][v_idx];
        }
    }
    for (unsigned int i=0; i<GRID_SIZE; i++){
        for (unsigned int j=0; j<GRID_SIZE; j++){
            for (unsigned int v_idx=0; v_idx<CANDIDATE_SIZE; v_idx++){
                m_grid_value_state[i][j][v_idx] = other.m_grid_value_state[i][j][v_idx];
            }
        }
    }
    
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        m_filled_count[i] = other.m_filled_count[i];
    }
};


void SolverV2::init_states(){
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            val_t filled_val = this->board().get_(i, j);
            if (filled_val == 0) continue;
            fill_propagate(i, j, filled_val);
        }
    }
};

OpState SolverV2::step_by_explicit_single(){
    bool updated = false;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            OpState state = update_by_explicit_single(i, j);
            if (state == OpState::SUCCESS){ 
                updated = true;
            }
            else if ( state == OpState::VIOLATION){
                return state;
            }
        }
    }
    return updated ? OpState::SUCCESS : OpState::FAIL;
};

OpState SolverV2::step_by_implicit_single(
    UnitType unit_type
){
    bool updated = false;
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        if (m_filled_count[i] == BOARD_SIZE) continue;  // the value was used up
        OpState state = update_by_implicit_single(i + 1, unit_type);
        if (state == OpState::SUCCESS){
            updated = true;
        }
        else if (state == OpState::VIOLATION){
            return state;
        }
    }
    return updated ? OpState::SUCCESS : OpState::FAIL;
}

bool SolverV2::step(){
    DEBUG_PRINT("SolverV2::step()");

    OpState state1 = step_by_explicit_single();
    if (state1 == OpState::VIOLATION) return false;
    if (state1 == OpState::SUCCESS) return true;
    DEBUG_PRINT("SolverV2::step() - step_by_only_candidate() failed");

    for (int i = 0; i < 3; i++)
    {
        UnitType unit_type = static_cast<UnitType>(i);
        OpState state2 = step_by_implicit_single(unit_type);
        if (state2 == OpState::VIOLATION) return false;
        if (state2 == OpState::SUCCESS) return true;
    }
    DEBUG_PRINT("SolverV2::step() - step_by_implicit_only_candidate() failed");

    if (USE_GUESS){
        OpState state3 = step_by_guess();
        if (state3 == OpState::SUCCESS) return true;
        DEBUG_PRINT("SolverV2::step() - step_by_guess() failed");
    }
    return false;
};

OpState SolverV2::fill_propagate(unsigned int row, unsigned int col, val_t value){
    board().get_(row, col) = value;

    // clear the candidates for the neighbor cells
    for (int i = 0; i < indexer.N_NEIGHBORS; i++){
        auto offset = indexer.neighbor_index[row][col][i];
        m_candidates.get(offset)[value - 1] = 0;
    }

    // update the filled count
    unsigned int v_idx = static_cast<unsigned int>(value) - 1;
    m_filled_count[v_idx] += 1;
    if (m_filled_count[v_idx] > BOARD_SIZE){
        return OpState::VIOLATION;
    }

    // update the unit fill state
    unsigned int grid_row = indexer.grid_lookup[row][col][0];
    unsigned int grid_col = indexer.grid_lookup[row][col][1];
    m_grid_value_state[grid_row][grid_col][v_idx] = 1;
    m_row_value_state[row][v_idx] = 1;
    m_col_value_state[col][v_idx] = 1;
    return OpState::SUCCESS;
};


bool SolverV2::refine_candidates(UnitType unit_type){
    // handles implicit value determination
    // i.e. if a sub-row/col in a grid has multiple candidates for a value,
    // but can uniquely determine the value based on the row/col 
    // (e.g. 57, 75, 375 appears in one row/col of a grid, determins 7 and 5 must be in the same row/col)
    // then we can remove the other candidates from the same total-row/col
    // to be implemented...
    return false;
};

OpState SolverV2::update_by_explicit_single(int row, int col){

    if (this->board().get_(row, col) != 0){ return OpState::SKIP; }

    val_t candidate_val = 0;
    OpState s = m_candidates.remain_x(row, col, 1, &candidate_val);
    if (s == OpState::VIOLATION){
        return OpState::VIOLATION;
    }
    if (s == OpState::SUCCESS){
        return fill_propagate(row, col, candidate_val);
    }
    return OpState::FAIL;
};

/*
This determines the value of a cell if 
it is the only cell in the row/col/grid that can have a certain value
*/
OpState SolverV2::update_by_implicit_single(val_t value, UnitType unit_type){

    auto solve_for_unit = [&](unsigned int* offset_start, unsigned int len){
        unsigned int candidate_count = 0;
        Coord candidate_coord;
        for (int i = 0; i < len; i++)
        {
            unsigned int offset = offset_start[i];
            val_t board_val = this->board().get(offset);
            if (board_val != 0) continue;                                     // skip filled cells
            if (this->m_candidates.get(offset)[value - 1] != 1) continue; // skip non-candidates
            candidate_coord.row = indexer.offset_coord_lookup[offset][0];
            candidate_coord.col = indexer.offset_coord_lookup[offset][1];
            candidate_count++;
            if (candidate_count > 1) break;
        }
        if (candidate_count == 1){
            return fill_propagate(candidate_coord.row, candidate_coord.col, value);
        }
        return OpState::FAIL;
    };

    unsigned int v_idx = value - 1;

    // check for implicit only candidate in the grids
    if (unit_type == UnitType::GRID){
        for (int g_i = 0; g_i < GRID_SIZE; g_i++)
        {
            for (int g_j = 0; g_j < GRID_SIZE; g_j++)
            {
                if (m_grid_value_state[g_i][g_j][v_idx] == 1){ continue; } // already filled
                // iterate through the grid
                unsigned int grid_start_row = g_i * GRID_SIZE;
                unsigned int grid_start_col = g_j * GRID_SIZE;
                OpState state = solve_for_unit(indexer.grid_index[grid_start_row][grid_start_col], GRID_SIZE * GRID_SIZE);
                // somehow must return here, instead of continue...
                // otherwise, benchmark.py will fail?
                if (state == OpState::SUCCESS || state == OpState::VIOLATION){
                    return state;
                }
            }
        }
    }

    // check for implicit only candidate in the rows and columns
    if (unit_type == UnitType::ROW){
        for (int r = 0; r < BOARD_SIZE; r++)
        {
            if (m_row_value_state[r][v_idx] == 1){ continue; } // already filled
            OpState state = solve_for_unit(indexer.row_index[r], BOARD_SIZE);
            if (state == OpState::SUCCESS || state == OpState::VIOLATION){
                return state;
            }
        }
    }

    if (unit_type == UnitType::COL){
        for (int c = 0; c < BOARD_SIZE; c++)
        {
            if (m_col_value_state[c][v_idx] == 1){ continue; } // already filled
            OpState state = solve_for_unit(indexer.col_index[c], BOARD_SIZE);
            if (state == OpState::SUCCESS || state == OpState::VIOLATION){
                return state;
            }
        }
    };

    return OpState::FAIL;
};

SolverV2 SolverV2::fork(){
    return SolverV2(*this);
};

OpState SolverV2::step_by_guess(){
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
    // 2. the cell with the largest number of unsolved neighbors (maximizing it's impact for quick feedback)
    auto get_heuristic_choice = [&]()->Coord {
        Coord best_choice;
        unsigned int min_candidate_count = 1e4;
        unsigned int max_neighbor_count = 1e4;
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                if (this->board().get_(i, j) != 0){ continue; };     // skip the solved cells

                unsigned int candidate_count = m_candidates.count(i, j);
                if (candidate_count < min_candidate_count){
                    min_candidate_count = candidate_count;
                    max_neighbor_count = numNeighborUnsolved(i, j);
                    best_choice = {i, j};
                }
                else if (candidate_count == min_candidate_count){
                    unsigned int neighbor_count = numNeighborUnsolved(i, j);
                    if (neighbor_count > max_neighbor_count){
                        max_neighbor_count = neighbor_count;
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
    val_t candidate_values[CANDIDATE_SIZE];
    unsigned int candidate_count = 0;

    // candidate_values.reserve(CANDIDATE_SIZE);
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        val_t val = static_cast<val_t>(i + 1);
        if (m_candidates.get_(best_choice.row, best_choice.col, val)){
            candidate_values[candidate_count] = val;
            candidate_count++;
        }
    }

    if (HEURISTIC_GUESS){
        // sort the candidate indices by the number of occurences in the board, 
        // starting with the one with the least occurences
        // this should facilitateos the backtracking process by increasing the value diversity, 
        // and reduce the chance of getting stuck in a local minimum (which requires more guesses)
        util::sort_array_bubble<val_t>(candidate_values, candidate_count);
    }
    else if (!DETERMINISTIC_GUESS){
        // shuffle the candidate indices
        util::shuffle_array<val_t>(candidate_values, candidate_count);
    }

    // make guesses with backtracking
    for (unsigned int i = 0; i < candidate_count; i++){
        val_t guess = candidate_values[i];

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
        return OpState::SUCCESS;
    }

    // ideally, we should never reach here...
    // unless the board is invalid, trail limit is reached, or the guess is wrong. 
    return OpState::FAIL;
};