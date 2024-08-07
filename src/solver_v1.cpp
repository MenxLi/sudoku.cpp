#include "config.h"
#include "solver_v1.h"

#define CANDIDATE_SIZE BOARD_SIZE

SolverV1::SolverV1(Board& board) : Solver(board) {
    clear_candidates();
};

bool SolverV1::step(){
    update_candidates();
    bool ret = update_values();
    if (ret){
        clear_candidates();
    }
    return ret;
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