#include "board.h"
#include "config.h"
#include "solver_base.hpp"
#include "solver.h"
#include <memory>
#include <random>

#include "x_double.cpp"
#include "x_xwing.cpp"

bool FillState::on_fill(unsigned int row, unsigned int col, val_t value){
    unsigned int v_idx = static_cast<unsigned int>(value) - 1;

    this -> count[v_idx]++;
    if (this -> count[v_idx] > BOARD_SIZE) { return false; }

    if (!this->row[row].test(v_idx)) { return false; }
    this->row[row].reset(v_idx);

    if (!this->col[col].test(v_idx)) { return false; }
    this->col[col].reset(v_idx);

    unsigned int grid_row = indexer.grid_lookup[row][col][0];
    unsigned int grid_col = indexer.grid_lookup[row][col][1];
    if (!this->grid[grid_row][grid_col].test(v_idx)) { return false; }
    this->grid[grid_row][grid_col].reset(v_idx);

    this->solved.set(indexer.coord_offset_lookup[row][col]);
    return true;
}


Solver::Solver(const Board& board) : 
    SolverBase(board), 
    m_config{std::make_shared<Solver_config>()}, 
    m_fstate{std::make_unique<FillState>()}
{ init_states(); };

Solver::Solver(Solver& other) noexcept : 
    SolverBase(other), 
    m_config{other.m_config}, 
    m_fstate{std::make_unique<FillState>(*other.m_fstate)}
{};

void Solver::init_states() {
    m_config = std::shared_ptr<Solver_config>(Solver_config::new_from_env());

    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            val_t filled_val = board().get(i, j).retrive();
            if (filled_val == 0) continue;
            fill_propagate(i, j, filled_val);
        }
    }
};

OpState Solver::step_by_naked_single() noexcept {
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

OpState Solver::step_by_hidden_single(
    UnitType unit_type
) noexcept {
    bool updated = false;
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        if (m_fstate->is_value_useup(i+1)) continue;
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

bool Solver::step(){
    DEBUG_PRINT("Solver::step()");

    auto step_by_single = [&]()->OpState{
        OpState state;
        state = step_by_naked_single();
        if (state == OpState::VIOLATION) return OpState::VIOLATION;
        if (state == OpState::SUCCESS) return OpState::SUCCESS;
        DEBUG_PRINT("Solver::step() - step_by_only_candidate() failed");

        for (unsigned int i = 0; i < 3; i++)
        {
            UnitType unit_type = static_cast<UnitType>(i);
            state = step_by_hidden_single(unit_type);
            if (state == OpState::VIOLATION) return OpState::VIOLATION;
            if (state == OpState::SUCCESS) return OpState::SUCCESS;
        }
        return OpState::FAIL;
    };

    OpState state;

    state = step_by_single();
    if (state == OpState::SUCCESS) return true;
    if (state == OpState::VIOLATION) return false;

    std::unique_ptr<Board> board_record = nullptr;
    if (config().use_double || config().use_xwing){
        board_record = std::make_unique<Board>(board());
    }
    auto state_from_tentative = [&](OpStateTentative t_state) -> OpState {
        if (t_state == OpStateTentative::MAYBE_SUCCESS) {
            if (board() == *board_record){ state = OpState::FAIL; }
            else { state = OpState::SUCCESS; }
        }
        else{
            state = opstate_from_tentative(t_state);
        }
        return state;
    };

    if (config().use_xwing){
        auto t_state = step_by_xwing();
        state = state_from_tentative(t_state);
        if (state == OpState::SUCCESS) return true;
        if (state == OpState::VIOLATION) return false;
    }

    if (config().use_double){
        auto t_state = step_by_double();
        state = state_from_tentative(t_state);
        if (state == OpState::SUCCESS) return true;
        if (state == OpState::VIOLATION) return false;
    }

    if (config().use_guess){
        state = step_by_guess();
        if (state == OpState::SUCCESS) return true;
    }
    return false;
};

OpState Solver::fill_propagate(unsigned int row, unsigned int col, val_t value) noexcept {
    board().set(row, col, value);
    bool success = m_fstate->on_fill(row, col, value);
    if (!success){
        return OpState::VIOLATION; // fill failed, invalid board
    }

    unsigned int v_idx = static_cast<unsigned int>(value) - 1;

    // clear the candidates for the neighbor cells
    for (unsigned int i = 0; i < indexer.N_NEIGHBORS; i++){
        auto offset = indexer.neighbor_index[row][col][i];
        if (m_fstate->is_cell_solved(offset)) continue; // skip solved cells

        auto& cell = board().get(offset);
        cell.reset(v_idx);
    }

    return OpState::SUCCESS;
};


OpState Solver::update_by_naked_single(unsigned int row, unsigned int col){

    Cell& c = board().get(row, col);
    if (m_fstate->is_cell_solved(row, col)){ return OpState::SKIP; }

    auto candidate_count = c.count();
    if (candidate_count == 0){
        return OpState::VIOLATION; // no candidate left, invalid board
    }
    if (candidate_count > 1){
        return OpState::FAIL; // more than one candidate left, cannot fill
    }
    val_t candidate_val = c.retrive_nocheck();
    return fill_propagate(row, col, candidate_val);
};

OpState Solver::update_by_hidden_single(val_t value, UnitType unit_type){
    auto solve_for_unit = [&](const unsigned int* offset_start){
        int find_offset = -1;
        for (unsigned int i = 0; i < UNIT_SIZE; i++)
        {
            unsigned int offset = offset_start[i];
            if (m_fstate->is_cell_solved(offset)) continue; // skip solved cells

            auto& cell = this->board().get(offset);
            if (!cell.test(value - 1)) continue; // skip non-candidates

            // if we already found a candidate, then this is not a hidden single
            if (find_offset != -1) return OpState::FAIL; 

            find_offset = static_cast<int>(offset);
        }
        if (find_offset == -1) {
            // should check the value is avaliable in the unit before calling
            return OpState::VIOLATION;
        }

        auto row = indexer.offset_coord_lookup[find_offset][0];
        auto col = indexer.offset_coord_lookup[find_offset][1];
        return fill_propagate(row, col, value);
    };

    // check for implicit only candidate in the grids
    if (unit_type == UnitType::GRID){
        for (unsigned int g_i = 0; g_i < GRID_SIZE; g_i++)
        {
            for (unsigned int g_j = 0; g_j < GRID_SIZE; g_j++)
            {
                if (m_fstate->is_in_grid(g_i, g_j, value)){ continue; } // already filled
                OpState state = solve_for_unit(indexer.grid_index[g_i][g_j]);
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
            if (m_fstate->is_in_row(r, value)){ continue; } // already filled
            OpState state = solve_for_unit(indexer.row_index[r]);
            if (state == OpState::SUCCESS || state == OpState::VIOLATION){
                return state;
            }
        }
    }

    if (unit_type == UnitType::COL){
        for (unsigned int c = 0; c < BOARD_SIZE; c++)
        {
            if (m_fstate->is_in_col(c, value)){ continue; } // already filled
            OpState state = solve_for_unit(indexer.col_index[c]);
            if (state == OpState::SUCCESS || state == OpState::VIOLATION){
                return state;
            }
        }
    };

    return OpState::FAIL;
};

OpState Solver::step_by_guess() noexcept {
    auto [best_choice, values_to_guess] = find_best_guess();

    // make guesses with backtracking
    for (auto guess : values_to_guess){

        this->iteration_counter().n_guesses += 1;

        auto forked_solver = Solver(*this);

        // inherit the iteration counter
        forked_solver.iteration_counter().limit = 
            this->iteration_counter().limit - this->iteration_counter().current;

        forked_solver.fill_propagate(best_choice.row, best_choice.col, guess);
        bool solved = forked_solver.solve();

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

std::pair<Coord, std::vector<val_t>> Solver::find_best_guess() noexcept {
    static thread_local std::random_device random_rd;
    static thread_local std::mt19937 random_gen(random_rd());

    auto n_neighbor_unsolved = [this](unsigned int row, unsigned int col)->unsigned int{
        unsigned int unsolved_count = 0;
        for (auto offset : indexer.neighbor_index[row][col]){
            if (!this->m_fstate->is_cell_solved(offset)){
                unsolved_count++;
            }
        }
        return unsolved_count;
    };

    // find the best cell to guess, 
    // by finding:
    // 1. the cell with the least number of candidates
    // 2. the cell with the largest number of unsolved neighbors (maximizing it's impact for quick feedback)
    auto get_heuristic_choice = [&]()->Coord {

        std::vector<Coord> best_choices;
        unsigned int min_candidate_count = 1e4;
        for (unsigned int i = 0; i < BOARD_SIZE; i++)
        {
            for (unsigned int j = 0; j < BOARD_SIZE; j++)
            {
                if (this->m_fstate->is_cell_solved(i, j)){ continue; }; // skip the solved cells
                unsigned int candidate_count = this->board().get(i, j).count();
                if (candidate_count < min_candidate_count) {
                    best_choices.clear();
                    min_candidate_count = candidate_count;
                    best_choices.push_back({i, j});
                }
                else if (candidate_count == min_candidate_count){
                    best_choices.push_back({i, j});
                }
            }
        }
        if (best_choices.size() == 1){ return best_choices[0]; }

        // compare the neighbor counts
        unsigned int max_neighbor_count = 0;
        Coord best_choice;
        for (auto choice : best_choices){
            unsigned int neighbor_count = n_neighbor_unsolved(choice.row, choice.col);
            if (neighbor_count > max_neighbor_count){
                max_neighbor_count = neighbor_count;
                best_choice = choice;
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
                    if (!this->m_fstate->is_cell_solved(i, j)){
                        unsolved_cells.push_back({i, j});
                    }
                }
            }
            // random guess
            std::uniform_int_distribution<> dis(0, unsolved_cells.size() - 1);
            int random_idx = dis(random_gen);
            best_choice = unsolved_cells[random_idx];
        }
        else{
            // choose the first unsolved cell
            bool _found = false;
            for (unsigned int i = 0; i < BOARD_SIZE; i++)
            {
                for (unsigned int j = 0; j < BOARD_SIZE; j++)
                {
                    if (!this->m_fstate->is_cell_solved(i, j)){
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

    // collect the candidates where the value is not 0
    struct CandidateFilledPair{
        val_t val;
        unsigned int count;
    };

    auto candidate_filled_pairs = std::vector<CandidateFilledPair>(CANDIDATE_SIZE);
    unsigned int candidate_count = 0;

    // candidate_values.reserve(CANDIDATE_SIZE);
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++)
    {
        val_t val = static_cast<val_t>(i + 1);
        if (this->board().get(best_choice.row, best_choice.col).test(i)){
            candidate_filled_pairs[candidate_count].val = val;
            candidate_filled_pairs[candidate_count].count = this->m_fstate->get_value_count(val);
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

    // return the best guess
    std::vector<val_t> candidate_values(candidate_count);
    for (unsigned int i = 0; i < candidate_count; i++){
        candidate_values[i] = candidate_filled_pairs[i].val;
    }

    return {best_choice, candidate_values};
}