#include "parser.hpp"
#include "board.h"
#include "config.h"
#include "solver.h"
#include "solver_v2.h"
#include <memory>

#define MAX_FORK_TRAIL MAX_ITER

// #define DEBUG_PRINT(x) std::cout << x << std::endl;
#define DEBUG_PRINT(x);

// initialize the static variables

SolverV2::SolverV2(const Board& board) : Solver(board), 
m_config(new SolverV2_config()), m_candidates{ new CandidateBoard() }, m_fill_state{ new FillState() }
{ init_states(); };

SolverV2::SolverV2(SolverV2& other) : Solver(other.board()), 
m_config(new SolverV2_config()), m_candidates{ new CandidateBoard() }, m_fill_state{ new FillState() }
{
    m_iteration_counter->load(*other.m_iteration_counter);
    m_fill_state->load(*other.m_fill_state);
    m_candidates->load(*other.m_candidates);
    m_config->load(*other.m_config);
};

void SolverV2::init_states(){
    *m_config = {
        parser::parse_env_i<bool>("SOLVER_USE_GUESS", true),
        parser::parse_env_i("SOLVER_DETERMINISTIC_GUESS", false),
        parser::parse_env_i("SOLVER_HEURISTIC_GUESS", true),
        parser::parse_env_i("SOLVER_USE_DOUBLE", false),
        false
    };
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            val_t filled_val = board().get_(i, j);
            if (filled_val == 0) continue;
            fill_propagate(i, j, filled_val);
        }
    }
};

SolverV2_config& SolverV2::config(){
    return *m_config;
};

OpState SolverV2::step_by_naked_single(){
    bool updated = false;
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            OpState state = update_by_naked_single(i, j);
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

OpState SolverV2::step_by_hidden_single(
    UnitType unit_type
){
    bool updated = false;
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        if (m_fill_state->count[i] == BOARD_SIZE) continue;  // the value was used up
        OpState state = update_by_hidden_single(i + 1, unit_type);
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

    auto step_by_single = [&]()->OpState{
        OpState state;
        state = step_by_naked_single();
        if (state == OpState::VIOLATION) return OpState::VIOLATION;
        if (state == OpState::SUCCESS) return OpState::SUCCESS;
        DEBUG_PRINT("SolverV2::step() - step_by_only_candidate() failed");

        for (unsigned int i = 0; i < 3; i++)
        {
            UnitType unit_type = static_cast<UnitType>(i);
            state = step_by_hidden_single(unit_type);
            if (state == OpState::VIOLATION) return OpState::VIOLATION;
            if (state == OpState::SUCCESS) return OpState::SUCCESS;
        }
        DEBUG_PRINT("SolverV2::step() - step_by_implicit_only_candidate() failed");
        return OpState::FAIL;
    };

    OpState state;

    state = step_by_single();
    if (state == OpState::VIOLATION) return false;
    if (state == OpState::SUCCESS) return true;

    // refine the candidates by naked double
    if (config().use_double){
        for (unsigned int i = 0; i < 3; i++)
        {
            UnitType unit_type = static_cast<UnitType>(i);
            state = refine_candidates_by_naked_double(unit_type);
            if (state == OpState::VIOLATION) return false;
            
            // try to solve the puzzle again
            iteration_counter().current += 1;
            state = step_by_single();
            // if (state == OpState::SUCCESS) std::cout << "Progressed after refining candidates by naked double" << std::endl;
            if (state == OpState::VIOLATION) return false;
            if (state == OpState::SUCCESS) return true;
        }

        for (unsigned int i = 0; i < 3; i++)
        {
            UnitType unit_type = static_cast<UnitType>(i);
            state = refine_candidates_by_hidden_double(unit_type);
            if (state == OpState::VIOLATION) return false;
            
            // try to solve the puzzle again
            iteration_counter().current += 1;
            state = step_by_single();
            // if (state == OpState::SUCCESS) std::cout << "Progressed after refining candidates by hidden double" << std::endl;
            if (state == OpState::VIOLATION) return false;
            if (state == OpState::SUCCESS) return true;
        }
    }

    if (config().use_guess){
        state = step_by_guess();
        if (state == OpState::SUCCESS) return true;
        DEBUG_PRINT("SolverV2::step() - step_by_guess() failed");
    }
    return false;
};

OpState SolverV2::fill_propagate(unsigned int row, unsigned int col, val_t value){
    // board().set(row, col, value);
    board().get_(row, col) = value;

    unsigned int v_idx = static_cast<unsigned int>(value) - 1;

    // clear the candidates for the neighbor cells
    for (unsigned int i = 0; i < indexer.N_NEIGHBORS; i++){
        auto offset = indexer.neighbor_index[row][col][i];
        (m_candidates->get(offset))[v_idx] = 0;
    }

    // update the filled count
    m_fill_state->count[v_idx] += 1;
    if (m_fill_state->count[v_idx] > BOARD_SIZE){
        return OpState::VIOLATION;
    }

    // update the unit fill state
    unsigned int grid_row = indexer.grid_lookup[row][col][0];
    unsigned int grid_col = indexer.grid_lookup[row][col][1];
    if (m_fill_state->grid[grid_row][grid_col][v_idx]){ return OpState::VIOLATION; }
    m_fill_state->grid[grid_row][grid_col][v_idx] = 1;

    if (m_fill_state->row[row][v_idx]){ return OpState::VIOLATION; }
    m_fill_state->row[row][v_idx] = 1;

    if (m_fill_state->col[col][v_idx]){ return OpState::VIOLATION; }
    m_fill_state->col[col][v_idx] = 1;

    return OpState::SUCCESS;
};


OpState SolverV2::refine_candidates_by_naked_double(UnitType unit_type){
    auto solve_for_unit = [&](const unsigned int* offset_start)->OpState{
        for (auto idx_pair : indexer.subunit_combinations_2){
            // initial validity check
            unsigned int offset1 = offset_start[idx_pair[0]];
            if (board().get(offset1) != 0) continue;        // skip filled cells

            unsigned int offset2 = offset_start[idx_pair[1]];
            if (board().get(offset2) != 0) continue;        // skip filled cells

            if (m_fill_state->visited_double_combinations[offset1][offset2] == 1) continue;

            // check if the two cells share the same candidates
            auto candidate_1 = m_candidates->get(offset1);
            auto candidate_2 = m_candidates->get(offset2);
            if (memcmp(candidate_1, candidate_2, CANDIDATE_SIZE*sizeof(decltype(candidate_1[0]))) != 0){
                continue;
            }

            // check if the first cell has only 2 candidates, the second cell is the same
            val_t double_values[2];
            OpState s1 = m_candidates->remain_x(offset1, 2, double_values);
            if (s1 == OpState::VIOLATION){ return OpState::VIOLATION; }
            if (s1 == OpState::FAIL){ continue; }

            m_fill_state->visited_double_combinations[offset1][offset2] = 1;

            // remove the double values from the other cells in the unit
            for (unsigned int i = 0; i < UNIT_SIZE; i++)
            {
                unsigned int offset = offset_start[i];
                if (offset == offset1 || offset == offset2) continue;
                if (board().get(offset) != 0) continue;        // skip filled cells
                for (val_t val : double_values){
                    m_candidates->get(offset)[val - 1] = 0;
                }
            }
        }
        return OpState::SUCCESS;
    };

    switch (unit_type)
    {
    case UnitType::ROW:
        for (unsigned int r = 0; r < BOARD_SIZE; r++)
        {
            OpState state = solve_for_unit(indexer.row_index[r]);
            if (state == OpState::VIOLATION){ return OpState::VIOLATION; }
        }
        break;
    case UnitType::COL:
        for (unsigned int c = 0; c < BOARD_SIZE; c++)
        {
            OpState state = solve_for_unit(indexer.col_index[c]);
            if (state == OpState::VIOLATION){ return OpState::VIOLATION; }
        }
        break;
    case UnitType::GRID:
        for (unsigned int g_i = 0; g_i < GRID_SIZE; g_i++)
        {
            for (unsigned int g_j = 0; g_j < GRID_SIZE; g_j++)
            {
                OpState state = solve_for_unit(indexer.grid_index[g_i][g_j]);
                if (state == OpState::VIOLATION){ return OpState::VIOLATION; }
            }
        }
        break;
    }
    return OpState::SUCCESS;
};

OpState SolverV2::refine_candidates_by_hidden_double(UnitType unit_type){
    auto solve_for_unit = [&](const unsigned int* offset_start, bool* unit_fill_state)->OpState{

        // array of candidates, each stores it's cell index in this unit
        util::SizedArray<unsigned int, UNIT_SIZE> unit_descriptor[CANDIDATE_SIZE];
        for (unsigned int i = 0; i < UNIT_SIZE; i++){
            const unsigned int offset = offset_start[i];
            if (board().get(offset) != 0) continue;        // skip filled cells
            // add each candidate to the corresponding array
            for (unsigned int v_idx = 0; v_idx < CANDIDATE_SIZE; v_idx++){
                if (m_candidates->get(offset)[v_idx]){
                    unit_descriptor[v_idx].push(i);
                }
            }
        }

        for (auto v_idx_pair : indexer.subvalue_combinations_2){
            // inital validity check
            // auto [v_idx1, v_idx2] = v_idx_pair;
            unsigned int v_idx1 = v_idx_pair[0];
            unsigned int v_idx2 = v_idx_pair[1];
            if (unit_fill_state[v_idx1] || unit_fill_state[v_idx2]) continue; // skip filled values
            if (unit_descriptor[v_idx1].size() != 2 || unit_descriptor[v_idx2].size() != 2) continue; // only consider hidden double
            if (!(unit_descriptor[v_idx1] == unit_descriptor[v_idx2])) continue; // only consider hidden double

            // remove other candidates from this two cells
            unsigned int offset_1 = offset_start[unit_descriptor[v_idx1][0]];
            unsigned int offset_2 = offset_start[unit_descriptor[v_idx1][1]];

            if (m_fill_state->visited_double_combinations[offset_1][offset_2] == 1) continue;
            m_fill_state->visited_double_combinations[offset_1][offset_2] = 1;

            bool_ aimed_one_hot[CANDIDATE_SIZE] = {0};
            aimed_one_hot[v_idx1] = 1;
            aimed_one_hot[v_idx2] = 1;

            std::memcpy(m_candidates->get(offset_1), aimed_one_hot, CANDIDATE_SIZE * sizeof(bool_));
            std::memcpy(m_candidates->get(offset_2), aimed_one_hot, CANDIDATE_SIZE * sizeof(bool_));

            // remove these two candidates from the other cells in the unit
            for (unsigned int i = 0; i < UNIT_SIZE; i++)
            {
                unsigned int offset = offset_start[i];
                if (offset == offset_1 || offset == offset_2) continue;
                if (board().get(offset) != 0) continue;        // skip filled cells
                m_candidates->get(offset)[v_idx1] = 0;
                m_candidates->get(offset)[v_idx2] = 0;
            }
        }
        return OpState::SUCCESS;
    };
    OpState state;
    switch(unit_type){
    case UnitType::ROW:
        for (unsigned int r = 0; r < BOARD_SIZE; r++)
        {
            state = solve_for_unit(indexer.row_index[r], m_fill_state->row[r]);
        }
        return state;
    case UnitType::COL:
        for (unsigned int c = 0; c < BOARD_SIZE; c++)
        {
            state = solve_for_unit(indexer.col_index[c], m_fill_state->col[c]);
        }
        return state;
    case UnitType::GRID:
        for (unsigned int g_i = 0; g_i < GRID_SIZE; g_i++)
        {
            for (unsigned int g_j = 0; g_j < GRID_SIZE; g_j++)
            {
                state = solve_for_unit(indexer.grid_index[g_i][g_j], m_fill_state->grid[g_i][g_j]);
            }
        }
        return state;
    default:
        return OpState::FAIL;   // should not reach here
    }
}

OpState SolverV2::update_by_naked_single(unsigned int row, unsigned int col){

    if (this->board().get(row, col)){ return OpState::SKIP; }

    val_t candidate_val = 0;
    OpState s = m_candidates->remain_x(row, col, 1, &candidate_val);
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
OpState SolverV2::update_by_hidden_single(val_t value, UnitType unit_type){

    auto solve_for_unit = [&](const unsigned int* offset_start){
        unsigned int candidate_count = 0;
        Coord candidate_coord;
        for (unsigned int i = 0; i < UNIT_SIZE; i++)
        {
            unsigned int offset = offset_start[i];
            val_t board_val = this->board().get(offset);
            if (board_val != 0) continue;                                     // skip filled cells
            if (this->m_candidates->get(offset)[value - 1] != 1) continue; // skip non-candidates
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
        for (unsigned int g_i = 0; g_i < GRID_SIZE; g_i++)
        {
            for (unsigned int g_j = 0; g_j < GRID_SIZE; g_j++)
            {
                if (m_fill_state->grid[g_i][g_j][v_idx]){ continue; } // already filled
                // iterate through the grid
                OpState state = solve_for_unit(indexer.grid_index[g_i][g_j]);
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
        for (unsigned int r = 0; r < BOARD_SIZE; r++)
        {
            if (m_fill_state->row[r][v_idx]){ continue; } // already filled
            OpState state = solve_for_unit(indexer.row_index[r]);
            if (state == OpState::SUCCESS || state == OpState::VIOLATION){
                return state;
            }
        }
    }

    if (unit_type == UnitType::COL){
        for (unsigned int c = 0; c < BOARD_SIZE; c++)
        {
            if (m_fill_state->col[c][v_idx]){ continue; } // already filled
            OpState state = solve_for_unit(indexer.col_index[c]);
            if (state == OpState::SUCCESS || state == OpState::VIOLATION){
                return state;
            }
        }
    };

    return OpState::FAIL;
};

OpState SolverV2::step_by_guess(){
    auto numNeighborUnsolved = [this](unsigned int row, unsigned int col)->unsigned int{
        unsigned int min_count;
        unsigned int row_count = 0;
        unsigned int col_count = 0;
        unsigned int grid_count = 0;

        auto row_item_offsets = indexer.row_index[row];
        auto col_item_offsets = indexer.col_index[col];
        auto grid_item_offsets = indexer.grid_coord_index[row][col];

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
    // 2. the cell with the largest number of unsolved neighbors (maximizing it's impact for quick feedback)
    auto get_heuristic_choice = [&]()->Coord {
        Coord best_choice;
        unsigned int min_candidate_count = 1e4;
        unsigned int max_neighbor_count = 1e4;
        for (unsigned int i = 0; i < BOARD_SIZE; i++)
        {
            for (unsigned int j = 0; j < BOARD_SIZE; j++)
            {
                if (this->board().get_(i, j) != 0){ continue; };     // skip the solved cells

                unsigned int candidate_count = m_candidates->count(i, j);
                if (candidate_count < min_candidate_count){
                    min_candidate_count = candidate_count;
                    max_neighbor_count = numNeighborUnsolved(i, j);
                    best_choice = {static_cast<int>(i), static_cast<int>(j)};
                }
                else if (candidate_count == min_candidate_count){
                    unsigned int neighbor_count = numNeighborUnsolved(i, j);
                    if (neighbor_count > max_neighbor_count){
                        max_neighbor_count = neighbor_count;
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
    // this value gaurentees the cell is not valid, and should be updated in the loop
    Coord best_choice {BOARD_SIZE, BOARD_SIZE};
    if (config().heuristic_guess){
        best_choice = get_heuristic_choice();
    }
    else{
        if (!config().deterministic_guess){
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
            srand(time(NULL));
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

    // collect the candidates where the value is not 0
    struct CandidateFilledPair{
        val_t val;
        unsigned int count;
    };

    auto candidate_filled_pairs = std::unique_ptr<CandidateFilledPair[]>(new CandidateFilledPair[CANDIDATE_SIZE]);
    unsigned int candidate_count = 0;

    // candidate_values.reserve(CANDIDATE_SIZE);
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        val_t val = static_cast<val_t>(i + 1);
        if (m_candidates->get_(best_choice.row, best_choice.col, val)){
            candidate_filled_pairs[candidate_count].val = val;
            candidate_filled_pairs[candidate_count].count = m_fill_state->count[i];
            candidate_count++;
        }
    }

    if (config().heuristic_guess){
        // sort the candidate indices by the number of occurences in the board, 
        // starting with the one with the least occurences
        // this should facilitateos the backtracking process by increasing the value diversity
        // but it seems not affecting the performance much...
        util::sort_array_bubble<CandidateFilledPair>(&candidate_filled_pairs[0], candidate_count, 
            [](CandidateFilledPair a, CandidateFilledPair b) { return a.count < b.count; }
            );
    }
    else if (!config().deterministic_guess){
        // shuffle the candidate indices
        util::shuffle_array<CandidateFilledPair>(&candidate_filled_pairs[0], candidate_count);
    }

    if (config().reverse_guess){
        // reverse the order of the candidates
        for (unsigned int i = 0; i < candidate_count / 2; i++){
            CandidateFilledPair temp = candidate_filled_pairs[i];
            candidate_filled_pairs[i] = candidate_filled_pairs[candidate_count - i - 1];
            candidate_filled_pairs[candidate_count - i - 1] = temp;
        }
    }

    // make guesses with backtracking
    for (unsigned int i = 0; i < candidate_count; i++){
        this->iteration_counter().n_guesses += 1;

        val_t guess = candidate_filled_pairs[i].val;

        auto forked_solver = SolverV2(*this);
        // auto forked_solver = *std::unique_ptr<SolverV2>(new SolverV2(*this));

        // inherit the iteration counter
        forked_solver.iteration_counter().current = this->iteration_counter().current;

        bool solved;
        forked_solver.fill_propagate(best_choice.row, best_choice.col, guess);
        solved = forked_solver.solve();

        this->iteration_counter().current = forked_solver.iteration_counter().current;
        this->iteration_counter().n_guesses = forked_solver.iteration_counter().n_guesses;

        if (!solved){ continue; }

        this->board().load_data(forked_solver.board());
        return OpState::SUCCESS;
    }

    // ideally, we should never reach here...
    // unless the board is invalid, trail limit is reached, or the guess is wrong. 
    return OpState::FAIL;
};