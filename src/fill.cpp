#include "fill.h"
#include "config.h"
#include <ctime>
#include <stack>

namespace gen {

    constexpr Board get_meta_board(){
        auto get_iota_row = [](){
            std::array<val_t, BOARD_SIZE> row_data;
            std::iota(row_data.begin(), row_data.end(), 1);
            return row_data;
        };

        auto lshift_row = [](std::array<val_t, BOARD_SIZE>& arr, unsigned int n){
            std::rotate(arr.begin(), arr.begin() + n, arr.end());
        };

        auto meta_row = [&](unsigned int row){
            std::array<val_t, BOARD_SIZE> row_data = get_iota_row();
            unsigned int n_shift = row / GRID_SIZE + (row % GRID_SIZE) * GRID_SIZE;
            lshift_row(row_data, n_shift);
            return row_data;
        };

        Board board;
        for (unsigned int i = 0; i < BOARD_SIZE; i++){
            auto row_data = meta_row(i);
            for (unsigned int j = 0; j < BOARD_SIZE; j++){
                board.set(i, j, row_data[j]);
            }
        }
        // ASSERT(board.is_solved(), "Invalid meta board");
        return board;
    }

    void apply_random_transform(Board& board, unsigned int n_repeats){
        auto transform_type_smplr = UniformIntDist<0, 3>();
        auto idx_smplr = UniformIntDist<0, GRID_SIZE - 1>();
        auto value_smplr = UniformIntDist<0, CANDIDATE_SIZE - 1>();
        for (unsigned int i = 0; i < n_repeats; i++){
            unsigned int transform_type = transform_type_smplr.sample();
            unsigned int idx1;
            unsigned int idx2;
            unsigned int g_idx1;
            unsigned int g_idx2;
            switch (transform_type){
                case 0:
                    idx1 = idx_smplr.sample();
                    idx2 = idx_smplr.sample();
                    g_idx1 = idx_smplr.sample();
                    BoardEquivalenceTransform::swap_row(board, g_idx1, idx1, idx2);
                    break;
                case 1:
                    g_idx1 = idx_smplr.sample();
                    g_idx2 = idx_smplr.sample();
                    BoardEquivalenceTransform::swap_band(board, g_idx1, g_idx2);
                    break;
                case 2:
                    idx1 = value_smplr.sample();
                    idx2 = value_smplr.sample();
                    BoardEquivalenceTransform::swap_value(board, idx1 + 1, idx2 + 1);
                    break;
                case 3:
                    BoardEquivalenceTransform::transpose(board);
                    break;
                default:
                    break;
            }
        }
        ASSERT(board.is_valid(), "Invalid board after applying random transform");
    }

    std::vector<val_t> get_candidates(Board& board, int row, int col){
        bool candidates_idx_allowd[CANDIDATE_SIZE];
        for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
            candidates_idx_allowd[i] = true;
        }
        for (auto offset: Board::indexer.neighbor_index[row][col]){
            val_t n_value = board.get(offset);
            if (n_value != 0){
                unsigned int v_idx = n_value - 1;
                candidates_idx_allowd[v_idx] = false;
            }
        }
        util::SizedArray<val_t, CANDIDATE_SIZE> result;
        for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
            if (candidates_idx_allowd[i]){
                result.push(i + 1);
            }
        }
        return std::vector<val_t>(result.data(), result.data() + result.size());
    };

    void fill_board_naive(Board &board){
        auto meta_board = get_meta_board();
        board.load_data(meta_board);
        apply_random_transform(board, 100 * BOARD_SIZE);
    }

    void fill_board_backtrack(Board& board){
        unsigned int offset = 0;
        
        struct StackItem{
            unsigned int offset;
            std::vector<val_t> candidates;
            unsigned int next_candidate_idx;
        };

        std::stack<StackItem> stack;

        // fill the first cell
        unsigned int row = Board::indexer.offset_coord_lookup[offset][0];
        unsigned int col = Board::indexer.offset_coord_lookup[offset][1];

        auto candidates = get_candidates(board, row, col);
        util::shuffle_array(candidates.data(), candidates.size());
        stack.push({offset, candidates, 0});

        while(stack.size() > 0){
            StackItem& top_item = stack.top();
            if (top_item.next_candidate_idx >= top_item.candidates.size()){
                // all candidates are tried, revert the current cell
                board.set(top_item.offset, 0);
                stack.pop();
                if (stack.size() == 0){
                    break;
                }
                stack.top().next_candidate_idx++;
                continue;
            }

            // fill the next cell
            val_t c = top_item.candidates[top_item.next_candidate_idx];
            board.set(top_item.offset, c);

            // check if the board is solved
            if (top_item.offset == CELL_COUNT - 1){
                ASSERT(board.is_solved(), "Invalid board, error while filling the board");
                return;
            }

            ASSERT(top_item.offset < CELL_COUNT - 1, "Invalid offset");

            // push the next cell to the stack
            offset = top_item.offset + 1;
            row = Board::indexer.offset_coord_lookup[offset][0];
            col = Board::indexer.offset_coord_lookup[offset][1];
            auto candidates = get_candidates(board, row, col);

            util::shuffle_array(candidates.data(), candidates.size());
            stack.push({offset, candidates, 0});
        }
    }

    void fill_board(Board &board, FillStrategy strategy){
        switch (strategy){

            case FillStrategy::NAIVE:
                fill_board_naive(board); break;

            case FillStrategy::BACKTRACK:
                board.clear(0);
                fill_board_backtrack(board); break;

            default:
                ASSERT(false, "Invalid fill strategy");
                return;
        }
    }

}