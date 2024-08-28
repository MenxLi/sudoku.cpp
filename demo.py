from sudoku_cpp import solve, generate, fmt_board, build_config
import argparse

if __name__ == "__main__":

    cell_count = build_config()['BOARD_SIZE']**2

    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--clues", type=int, default=cell_count//2, help="Number of clues")
    args = parser.parse_args()

    gen = generate(args.clues, parallel_exec=True)
    print(gen, end='\n\n')
    puzzle = gen['data']
    print("Puzzle:")
    print(fmt_board(puzzle), end='\n\n')

    solution = solve(puzzle)
    print(solution, end='\n\n')
    solved_puzzle = solution['data']
    print("Solution:")
    print(fmt_board(solved_puzzle), end='\n\n')