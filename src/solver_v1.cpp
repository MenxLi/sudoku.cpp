#include "config.h"
#include "solver_v1.h"

#define CANDIDATE_SIZE BOARD_SIZE

// #define DEBUG_PRINT(x) std::cout << x << std::endl;
#define DEBUG_PRINT(x);
static std::string _fmt_cross_map(val_t m_cross_map[BOARD_SIZE][BOARD_SIZE]){
    std::string ret = "";
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            ret += std::to_string(static_cast<int>(m_cross_map[i][j])) + " ";
        }
        ret += "\n";
    }
    return ret;
};

SolverV1::SolverV1(Board& board) : Solver(board) {
    clear_candidates();
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

bool SolverV1::step(){
    DEBUG_PRINT("SolverV1::step()");
    if (step_by_candidate()) return true;
    DEBUG_PRINT("SolverV1::step() - step_by_candidate() failed");
    if (step_by_crossover()) return true;
    DEBUG_PRINT("SolverV1::step() - step_by_crossover() failed");
    return false;
};

void SolverV1::update_candidate_for(int row, int col){
    Cell& cell = this->cell(row, col);
    if (cell.value() != 0){
        // already has a value
        return;
    }

    // remove candidates from the same row, column, and grid
    for (int idx = 0; idx < BOARD_SIZE; idx++)
    {
        val_t other_val = *cell.row()[idx];
        if (other_val != 0){
            m_candidates[row][col][other_val - 1] = 0;
        }
    }
    for (int idx = 0; idx < BOARD_SIZE; idx++)
    {
        val_t other_val = *cell.col()[idx];
        if (other_val != 0){
            m_candidates[row][col][other_val - 1] = 0;
        }
    }
    for (int idx = 0; idx < BOARD_SIZE; idx++)
    {
        val_t other_val = *cell.grid()[idx];
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

    // std::cout << "Cell: " << cell.coord().col << ", " << cell.coord().row << " has " << candidate_count << " candidates" << std::endl;
    ASSERT(candidate_count != 0, "No candidate left for cell, bad board?");

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
    for (unsigned int row_idx = 0; row_idx < BOARD_SIZE; row_idx++)
    {
        #ifdef STRICT
        unsigned int _count = 0;
        #endif

        for (unsigned int col_idx = 0; col_idx < BOARD_SIZE; col_idx++)
        {
            if (m_cross_map[row_idx][col_idx] == 1){
                // found a cell with the value
                // fill the entire row with the 1
                for (unsigned int i = 0; i < BOARD_SIZE; i++)
                {
                    m_cross_map_row[row_idx][i] = 1;
                }

                #ifdef STRICT
                _count++;
                #else
                break;
                #endif
            }

            #ifdef STRICT
            ASSERT(_count <= 1, "More than one cell with same value in the row");
            #endif
        }
    }

    for (unsigned int col_idx = 0; col_idx < BOARD_SIZE; col_idx++)
    {
        #ifdef STRICT
        unsigned int _count = 0;
        #endif

        for (unsigned int row_idx = 0; row_idx < BOARD_SIZE; row_idx++)
        {
            if (m_cross_map[row_idx][col_idx] == 1){
                // found a cell with the value
                // fill the entire column with the 1
                for (unsigned int i = 0; i < BOARD_SIZE; i++)
                {
                    m_cross_map_col[i][col_idx] = 1;
                }

                #ifdef STRICT
                _count++;
                #else
                break;
                #endif
            }
            #ifdef STRICT
            ASSERT(_count <= 1, "More than one cell with same value in the column");
            #endif
        }
    }
    
    // update the cross map with the row and column maps
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            m_cross_map[i][j] = m_cross_map_row[i][j] == 1 || m_cross_map_col[i][j] == 1;
        }
    }

    // std::cout << "Cross map for value " << static_cast<int>(value) << std::endl;
    // std::cout << "----------------" << std::endl;
    DEBUG_PRINT("Cross map for value " << static_cast<int>(value));
    DEBUG_PRINT("----------------");
    DEBUG_PRINT(_fmt_cross_map(m_cross_map));
    DEBUG_PRINT("Current board");
    DEBUG_PRINT("----------------");
    DEBUG_PRINT(this->board());

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
                DEBUG_PRINT("Found a cell to fill in: " << row_base + aim_grid_row_idx << ", " << col_base + aim_grid_col_idx << " with value " << static_cast<int>(value));
            }
        }
    }
    return ret;
};

void SolverV1::clear_cross_map(){
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            m_cross_map[i][j] = 0;
            m_cross_map_row[i][j] = 0;
            m_cross_map_col[i][j] = 0;
        }
    }
};
